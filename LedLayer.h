#pragma once
/*
 * LedLayer — Layered LED Display Library
 *
 * This header implements a flexible framework for composing multiple
 * independent information streams on a single addressable LED strip or ring.
 * It follows the architecture documented in docs/architecture.md of the
 * LedLayer repository.  Users define logical "layers" bound to their
 * application variables or sensors.  Each layer specifies how the raw
 * input should be mapped (range, clamping or wrapping) and filtered
 * (EMA smoothing, hysteresis) before being encoded by a chosen "mode".
 * Modes are visual encodings that write into dedicated output tracks
 * (color, brightness, mask, motion or overlay).  A layout‑aware renderer
 * combines the tracks into pixels, handling linear strips and circular
 * rings transparently.  Notifications provide temporary overrides for
 * alerts without disturbing the baseline composition.
 *
 * This implementation intentionally covers only a subset of the design
 * described in the architecture document.  It establishes the core
 * abstractions (layouts, layers, modes, tracks, renderer and
 * notifications) and provides a handful of concrete modes (palette
 * colors, gradient colors, global brightness, fill bars and a simple
 * chase pattern) as a proof of concept.  Additional modes, tracks and
 * layouts can be added later without breaking the API.
 */

#include <Arduino.h>
#include <FastLED.h>

namespace LedLayer {

/* ------------------------------------------------------------------
 * Layouts
 *
 * A Layout defines how normalized positions in the logical domain
 * [0..1] map to physical LED indices.  LinearLayout clamps at the
 * ends while RingLayout wraps around, with configurable starting
 * offsets and directions.  Users may derive new layouts for custom
 * geometries (spirals, segmented rings, matrices, etc.) by overriding
 * indexFrom01() and size().
 */

class Layout {
public:
    virtual ~Layout() = default;
    /** Return the number of physical LEDs. */
    virtual uint16_t size() const = 0;
    /** True if the geometry wraps (ring). */
    virtual bool wraps() const = 0;
    /** Map a normalized position t ∈ [0,1] to a physical LED index. */
    virtual uint16_t indexFrom01(float t) const = 0;
};

class LinearLayout : public Layout {
public:
    explicit LinearLayout(uint16_t count) : _count(count) {}
    uint16_t size() const override { return _count; }
    bool wraps() const override { return false; }
    uint16_t indexFrom01(float t) const override {
        if (_count == 0) return 0;
        // clamp t to [0,1]
        if (t < 0.0f) t = 0.0f;
        else if (t > 1.0f) t = 1.0f;
        // map such that t==1 maps to last LED
        float pos = t * float(_count - 1);
        int32_t i = (int32_t)lroundf(pos);
        if (i < 0) i = 0;
        if (i >= (int32_t)_count) i = (int32_t)_count - 1;
        return uint16_t(i);
    }
private:
    uint16_t _count;
};

class RingLayout : public Layout {
public:
    /**
     * Construct a ring layout with n LEDs.  The optional offset
     * specifies which physical index corresponds to logical position
     * t=0.0.  Direction controls whether increasing t moves clockwise
     * (true) or counter‑clockwise (false).
     */
    RingLayout(uint16_t count, uint16_t offset = 0, bool clockwise = true)
        : _count(count), _offset(offset), _clockwise(clockwise) {}
    uint16_t size() const override { return _count; }
    bool wraps() const override { return true; }
    uint16_t indexFrom01(float t) const override {
        if (_count == 0) return 0;
        // wrap t into [0,1)
        t = t - floorf(t);
        float pos = t * float(_count); // [0..n)
        int32_t i = int32_t(floorf(pos));
        if (i >= (int32_t)_count) i = 0;
        int32_t phys;
        if (_clockwise) {
            phys = i;
        } else {
            phys = int32_t(_count) - 1 - i;
        }
        phys += int32_t(_offset);
        phys %= int32_t(_count);
        if (phys < 0) phys += _count;
        return uint16_t(phys);
    }
private:
    uint16_t _count;
    uint16_t _offset;
    bool _clockwise;
};

/* ------------------------------------------------------------------
 * Tracks and Modes
 *
 * A Mode encodes how a standardized layer value (continuous or
 * discrete) is translated into a specific output track.  Each mode
 * belongs to exactly one track.  Track rules define whether multiple
 * modes can contribute (combinable) or whether only a single mode
 * can be active (exclusive).
 */

enum class TrackType : uint8_t {
    COLOR,
    BRIGHTNESS,
    MASK,
    MOTION,
    OVERLAY
};

// Enumeration of the supported modes.  Many of these are not yet
// implemented in the reference implementation below; unsupported
// modes will be ignored at runtime.  Additional modes can be added
// as needed.
enum class ModeType : uint8_t {
    // Color modes (COLOR track, exclusive)
    COLOR_STATE_PALETTE,    // discrete state → palette index
    COLOR_BINARY,           // boolean → on/off color
    COLOR_VALUE_GRADIENT,   // continuous value → gradient
    COLOR_VALUE_HUE,        // continuous value → hue wheel
    COLOR_CATEGORY_PALETTE, // category → theme palette (not implemented)

    // Brightness modes (BRIGHTNESS track, combinable)
    BRIGHTNESS_VALUE,       // continuous value → global brightness
    BRIGHTNESS_BINARY,      // boolean dim/boost
    BRIGHTNESS_GAMMA,       // continuous value with gamma
    BRIGHTNESS_LIMITER,     // cap brightness via ambient (not implemented)

    // Mask modes (MASK track, exclusive)
    MASK_FILL,              // value → bar/arc fill length
    MASK_CENTER_FILL,       // value → symmetric fill (not implemented)
    MASK_WINDOW_POSITION,   // value → sliding window position (not impl.)
    MASK_TICK_COUNT,        // discrete tick marks (not implemented)
    MASK_SEGMENT_ENABLE,    // boolean → region enable (not implemented)
    MASK_DENSITY,           // value → density (not implemented)

    // Motion modes (MOTION track, exclusive pattern + optional speed)
    MOTION_SOLID,           // static
    MOTION_PULSE,           // breathing pulse (not implemented)
    MOTION_BLINK,           // blink (not implemented)
    MOTION_CHASE,           // chasing segment
    MOTION_SCANNER,         // scanner (not implemented)
    MOTION_TWINKLE,         // twinkle (not implemented)
    MOTION_SPEED,           // secondary speed modulation (not implemented)

    // Overlay modes (OVERLAY track, combinable)
    OVERLAY_MARKER_SINGLE,  // marker at a normalized position
    OVERLAY_MARKER_THICK,   // thicker marker (not implemented)
    OVERLAY_THRESHOLD_MARKS,// threshold markers (not implemented)
    OVERLAY_CLOCK_HANDS,    // clock hands (not implemented)
    OVERLAY_CARDINAL_TICKS  // static ticks (not implemented)
};

/* ------------------------------------------------------------------
 * Layer configuration
 *
 * A Layer binds an application variable or sensor to a Mode.  It
 * defines how to map the raw input into a normalized form (0..1 or
 * discrete state), optionally filters it (EMA smoothing, hysteresis)
 * and then encodes it via the chosen mode.  The union of all active
 * layers forms the per‑frame track state.
 */

struct LayerConfig {
    // Pointer to the raw input variable.  The library treats the data
    // behind this pointer as a float representing the underlying
    // quantity.  Discrete values should also be provided as floats
    // (e.g. 0,1,2).  When using a boolean, cast to float and assign
    // either 0.0f or 1.0f.  For other types, consider supplying a
    // custom callback to pre‑process the data externally.
    const float* source = nullptr;

    // Mapping from raw input to standardized range.  inMin and
    // inMax define the expected raw range.  If clamp is true,
    // values outside the range are clamped; if clamp is false and
    // wrap is true, values wrap around; otherwise the result is
    // allowed to exceed 0..1.
    float inMin = 0.0f;
    float inMax = 1.0f;
    bool clamp = true;
    bool wrap = false;

    // Filtering options.  If emaAlpha > 0, an exponential moving
    // average with smoothing factor alpha is applied to the
    // standardized value.  For discrete layers (e.g. color
    // palette), hysteresis can be specified to avoid rapid state
    // toggling; hystBand defines the half‑width of the hysteresis
    // band in normalized units.  Both filters operate on the
    // standardized value; EMA is applied first.
    bool emaEnabled = false;
    float emaAlpha = 0.1f;      // smoothing factor (0.0 = no smoothing)
    mutable float emaState = 0; // internal filter state (mutable for const functions)

    bool hystEnabled = false;
    float hystBand = 0.05f;     // half bandwidth for hysteresis on thresholds
    mutable float hystState = 0; // last stable discrete value

    // Mode assigned to this layer.  Determines which track it writes.
    ModeType mode = ModeType::COLOR_STATE_PALETTE;

    // Mode parameters.  Use only the field relevant to the selected
    // mode.  For palette modes, define a small palette.  For
    // gradient modes, define start and end colours.  For brightness
    // modes, gamma can be specified.  For mask modes, define
    // optional start offset.  For motion modes, segment length,
    // pattern colour and speed factor may be provided.  Overlay
    // modes use a normalized position and optional thickness.
    struct PaletteParam {
        uint8_t count = 0;
        CRGB colors[8];
    } palette;
    struct GradientParam {
        CRGB from = CRGB::Black;
        CRGB to   = CRGB::White;
    } gradient;
    struct BrightnessParam {
        float gamma = 1.0f;
    } brightness;
    struct MaskParam {
        float start = 0.0f;    // normalized start position for fills
    } mask;
    struct MotionParam {
        uint8_t segmentPixels = 3; // for chase pattern
        CRGB color = CRGB::White;
        float speed = 1.0f;       // relative speed (0..?)
    } motion;
    struct OverlayParam {
        float pos = 0.0f;      // normalized position of marker
        CRGB color = CRGB::White;
        uint8_t thickness = 1; // number of pixels (only for linear layout)
    } overlay;

    // Priority for resolving conflicts on exclusive tracks.  Higher
    // priority wins when two layers target the same exclusive track.
    int priority = 0;
};

/* ------------------------------------------------------------------
 * Internal structures for track state
 */

// Colour track state.  Only one layer can write to this at a time.
struct ColorTrack {
    bool active = false;
    CRGB color = CRGB::Black;
};

// Brightness track state.  Multiple layers combine multiplicatively.
struct BrightnessTrack {
    bool active = false;
    float scale = 1.0f; // 0..1 multiplicative dimming
};

// Mask track state.  Only one layer can write to this at a time.
struct MaskTrack {
    bool active = false;
    // For fill modes: fill amount in [0..1]; for centre fill the
    // interpretation differs (not implemented yet).  We also store
    // start offset; centre fill uses start as centre.  When active
    // is false, the mask is considered fully lit.
    float start = 0.0f;
    float amount = 1.0f;
};

// Motion track state.  Only one layer can write to this at a time.
struct MotionTrack {
    bool active = false;
    ModeType pattern = ModeType::MOTION_SOLID;
    uint8_t segmentPixels = 1;
    CRGB color = CRGB::White;
    float speed = 1.0f;
};

// Overlay track state.  Multiple overlays accumulate.  For
// simplicity we support up to a fixed number of markers in one
// frame; this should suffice for most indicators.
static const uint8_t MAX_OVERLAYS = 8;
struct OverlayMarker {
    float pos = 0.0f;
    CRGB color = CRGB::White;
    uint8_t thickness = 1;
};
struct OverlayTrack {
    uint8_t count = 0;
    OverlayMarker markers[MAX_OVERLAYS];
};

/* ------------------------------------------------------------------
 * Notification system
 *
 * Notifications temporarily override the current composition with a
 * simple pattern.  When active, notifications can either overlay on
 * top of the existing frame (overlay mode) or completely replace it
 * (override mode).  After the duration expires, the notification
 * ends and the underlying composition resumes.  Notifications can
 * queue; higher priority notifications supersede lower priority.
 */

enum class NotifType : uint8_t {
    FLASH,
    PULSE,
    CHASE
};

enum class NotifMode : uint8_t {
    OVERRIDE,
    OVERLAY
};

struct Notification {
    NotifType type = NotifType::FLASH;
    NotifMode mode = NotifMode::OVERRIDE;
    CRGB color = CRGB::White;
    uint32_t startMs = 0;
    uint32_t durationMs = 500;
    uint8_t priority = 0;
    // Additional parameter: period (for flash), segment length (for chase)
    uint16_t param = 200;
};

/* ------------------------------------------------------------------
 * Main Display class
 *
 * This class manages a CRGB array, a Layout, a collection of
 * LayerConfig instances, track composition, rendering, and
 * notifications.  Call addLayer() to register layers, begin() to
 * validate and attach filters, then repeatedly call tick() with
 * millis() to update and render the display.
 */

template<uint8_t MAX_LAYERS = 8, uint8_t MAX_NOTIFS = 4>
class Display {
public:
    /**
     * Create a new Display.  The user supplies an array of CRGB
     * objects (leds) and a Layout instance describing the physical
     * arrangement.  The Display does not allocate memory for the
     * LEDs; it operates directly on the provided buffer.  The
     * FastLED library should be initialised elsewhere (in setup()).
     */
    Display(CRGB* leds, Layout& layout)
        : _leds(leds), _layout(layout) {}

    /**
     * Register a layer.  Returns false if the maximum number of
     * layers has been reached.  Layers are processed in the order
     * they are added.  Priority is only used to resolve conflicts
     * between layers targeting the same exclusive track.
     */
    bool addLayer(const LayerConfig& cfg) {
        if (_layerCount >= MAX_LAYERS) return false;
        _layers[_layerCount++] = cfg;
        return true;
    }

    /**
     * Validate the configured layers.  Checks for multiple
     * exclusive modes targeting the same track and attaches
     * hysteresis state.  Returns true on success or false on
     * conflict.  If conflicting layers exist, the user should
     * adjust priorities or remove one.  This should be called in
     * setup() after all layers are added.
     */
    bool begin() {
        // Reset track ownership info
        bool colorClaimed = false;
        int colorPri = -32768;
        bool maskClaimed = false;
        int maskPri = -32768;
        bool motionClaimed = false;
        int motionPri = -32768;
        // Check conflicts
        for (uint8_t i = 0; i < _layerCount; ++i) {
            const LayerConfig& cfg = _layers[i];
            TrackType track = modeToTrack(cfg.mode);
            bool exclusive = isExclusiveTrack(track);
            if (exclusive) {
                switch (track) {
                    case TrackType::COLOR:
                        if (colorClaimed && cfg.priority == colorPri) return false;
                        if (cfg.priority > colorPri) {
                            colorClaimed = true;
                            colorPri = cfg.priority;
                        }
                        break;
                    case TrackType::MASK:
                        if (maskClaimed && cfg.priority == maskPri) return false;
                        if (cfg.priority > maskPri) {
                            maskClaimed = true;
                            maskPri = cfg.priority;
                        }
                        break;
                    case TrackType::MOTION:
                        if (motionClaimed && cfg.priority == motionPri) return false;
                        if (cfg.priority > motionPri) {
                            motionClaimed = true;
                            motionPri = cfg.priority;
                        }
                        break;
                    default:
                        break;
                }
            }
            // initialise hysteresis state to zero or the current discrete value
            _layers[i].hystState = 0.0f;
            // EMA state initialised on first update
        }
        return true;
    }

    /**
     * Queue a notification.  If no active notification exists or
     * the new notification has a priority greater or equal to the
     * current one, it becomes active immediately and any queued
     * notifications are cleared.  Otherwise it is appended to the
     * queue (if space permits).  Return false on queue overflow.
     */
    bool notify(const Notification& notif) {
        Notification n = notif;
        n.startMs = _now;
        if (!_notifActive) {
            _activeNotif = n;
            _notifActive = true;
            _notifQueueCount = 0;
            return true;
        }
        if (n.priority >= _activeNotif.priority) {
            _activeNotif = n;
            _notifActive = true;
            _notifQueueCount = 0;
            return true;
        }
        if (_notifQueueCount >= MAX_NOTIFS) return false;
        _notifQueue[_notifQueueCount++] = n;
        return true;
    }

    /**
     * Update the display.  Pass the current time in milliseconds.
     * This function reads each layer's input, applies mapping and
     * filtering, composes the track state, renders the result to
     * the LED buffer, and applies any active notification.  Call
     * FastLED.show() after tick() to flush the LEDs.
     */
    void tick(uint32_t nowMs) {
        _now = nowMs;
        // Process notifications
        if (_notifActive) {
            uint32_t elapsed = nowMs - _activeNotif.startMs;
            if (elapsed >= _activeNotif.durationMs) {
                // Notification ended
                _notifActive = false;
                if (_notifQueueCount > 0) {
                    _activeNotif = _notifQueue[0];
                    for (uint8_t i = 1; i < _notifQueueCount; ++i) {
                        _notifQueue[i - 1] = _notifQueue[i];
                    }
                    _notifQueueCount--;
                    _activeNotif.startMs = nowMs;
                    _notifActive = true;
                }
            }
        }
        // Clear track state
        ColorTrack colorTrack;
        BrightnessTrack brightnessTrack;
        MaskTrack maskTrack;
        MotionTrack motionTrack;
        OverlayTrack overlayTrack;
        // Compose layers
        for (uint8_t i = 0; i < _layerCount; ++i) {
            const LayerConfig& cfg = _layers[i];
            if (!cfg.source) continue;
            float raw = *cfg.source;
            // Map raw to [0..1] or discrete state
            float mapped = 0.0f;
            if (cfg.inMax != cfg.inMin) {
                mapped = (raw - cfg.inMin) / (cfg.inMax - cfg.inMin);
            }
            if (cfg.wrap) {
                mapped = mapped - floorf(mapped);
            } else if (cfg.clamp) {
                if (mapped < 0.0f) mapped = 0.0f;
                if (mapped > 1.0f) mapped = 1.0f;
            }
            // Apply EMA smoothing if enabled
            float val = mapped;
            if (cfg.emaEnabled) {
                if (i >= 0) { // ensure state initialisation
                    if (cfg.emaState == 0 && nowMs == _now) {
                        // If emaState is zero at initial tick, initialise to first value
                        const_cast<LayerConfig&>(cfg).emaState = val;
                    }
                    float state = cfg.emaState;
                    state = state + cfg.emaAlpha * (val - state);
                    const_cast<LayerConfig&>(cfg).emaState = state;
                    val = state;
                }
            }
            // Apply hysteresis for discrete values
            float discVal = val;
            if (cfg.hystEnabled) {
                float prev = cfg.hystState;
                float half = cfg.hystBand;
                // If val is within half band around prev, keep prev
                if (fabsf(val - prev) <= half) {
                    discVal = prev;
                } else {
                    discVal = (val > prev) ? 1.0f : 0.0f; // snap to extremes
                    const_cast<LayerConfig&>(cfg).hystState = discVal;
                }
            }
            // Determine track and encode
            TrackType track = modeToTrack(cfg.mode);
            switch (track) {
                case TrackType::COLOR: {
                    // Use highest priority layer; skip if lower
                    if (!colorTrack.active || cfg.priority >= _maxColorPri) {
                        CRGB c = CRGB::Black;
                        switch (cfg.mode) {
                            case ModeType::COLOR_STATE_PALETTE: {
                                // Interpret discVal as state index (0..N-1)
                                uint8_t idx = uint8_t(discVal + 0.5f);
                                if (idx < cfg.palette.count) {
                                    c = cfg.palette.colors[idx];
                                } else if (cfg.palette.count > 0) {
                                    c = cfg.palette.colors[cfg.palette.count - 1];
                                }
                            } break;
                            case ModeType::COLOR_VALUE_GRADIENT: {
                                // Linear interpolate between gradient.from and gradient.to
                                float t = val;
                                uint8_t r = uint8_t((1.0f - t) * cfg.gradient.from.r + t * cfg.gradient.to.r);
                                uint8_t g = uint8_t((1.0f - t) * cfg.gradient.from.g + t * cfg.gradient.to.g);
                                uint8_t b = uint8_t((1.0f - t) * cfg.gradient.from.b + t * cfg.gradient.to.b);
                                c = CRGB(r, g, b);
                            } break;
                            case ModeType::COLOR_VALUE_HUE: {
                                uint8_t h = uint8_t(val * 255.0f);
                                c = CHSV(h, 255, 255);
                            } break;
                            case ModeType::COLOR_BINARY: {
                                c = val >= 0.5f ? cfg.gradient.to : cfg.gradient.from;
                            } break;
                            default:
                                // unsupported color mode
                                break;
                        }
                        colorTrack.active = true;
                        colorTrack.color = c;
                        _maxColorPri = cfg.priority;
                    }
                } break;
                case TrackType::BRIGHTNESS: {
                    // Combine multiplicatively
                    float scale = 1.0f;
                    switch (cfg.mode) {
                        case ModeType::BRIGHTNESS_VALUE: {
                            scale = val;
                        } break;
                        case ModeType::BRIGHTNESS_BINARY: {
                            scale = val >= 0.5f ? 1.0f : 0.0f;
                        } break;
                        case ModeType::BRIGHTNESS_GAMMA: {
                            // Gamma parameter in cfg.brightness.gamma
                            float g = cfg.brightness.gamma;
                            scale = powf(val, g);
                        } break;
                        default:
                            break;
                    }
                    brightnessTrack.active = true;
                    brightnessTrack.scale *= scale;
                } break;
                case TrackType::MASK: {
                    if (!maskTrack.active || cfg.priority >= _maxMaskPri) {
                        float start = cfg.mask.start;
                        float amount = val;
                        if (amount < 0.0f) amount = 0.0f;
                        if (amount > 1.0f) amount = 1.0f;
                        maskTrack.active = true;
                        maskTrack.start = start;
                        maskTrack.amount = amount;
                        _maxMaskPri = cfg.priority;
                    }
                } break;
                case TrackType::MOTION: {
                    if (!motionTrack.active || cfg.priority >= _maxMotionPri) {
                        if (cfg.mode == ModeType::MOTION_CHASE) {
                            motionTrack.pattern = cfg.mode;
                            motionTrack.segmentPixels = cfg.motion.segmentPixels;
                            motionTrack.color = cfg.motion.color;
                            // Use val to modulate speed: val=0..1 → slow..fast
                            motionTrack.speed = cfg.motion.speed * (0.2f + val * 2.0f);
                        } else {
                            motionTrack.pattern = ModeType::MOTION_SOLID;
                            motionTrack.segmentPixels = 0;
                            motionTrack.speed = 0;
                        }
                        motionTrack.active = true;
                        _maxMotionPri = cfg.priority;
                    }
                } break;
                case TrackType::OVERLAY: {
                    if (overlayTrack.count < MAX_OVERLAYS) {
                        OverlayMarker marker;
                        if (cfg.mode == ModeType::OVERLAY_MARKER_SINGLE) {
                            marker.pos = cfg.overlay.pos;
                            marker.color = cfg.overlay.color;
                            marker.thickness = cfg.overlay.thickness;
                            overlayTrack.markers[overlayTrack.count++] = marker;
                        }
                    }
                } break;
            }
        }
        // Render composition into LED buffer
        uint16_t n = _layout.size();
        // default colour if none selected
        CRGB baseColor = colorTrack.active ? colorTrack.color : CRGB::Black;
        float globalBright = brightnessTrack.active ? brightnessTrack.scale : 1.0f;
        if (globalBright < 0.0f) globalBright = 0.0f;
        if (globalBright > 1.0f) globalBright = 1.0f;
        // Precompute motion phase for chase pattern
        float chasePos01 = 0.0f;
        if (motionTrack.active && motionTrack.pattern == ModeType::MOTION_CHASE && n > 0) {
            // Use time and speed: one full revolution per 2000ms by default
            float period = 2000.0f / motionTrack.speed;
            float frac = fmodf(float(nowMs % (uint32_t)period), period) / period;
            chasePos01 = frac;
        }
        for (uint16_t i = 0; i < n; ++i) {
            // compute normalized coordinate on [0..1)
            float t;
            if (_layout.wraps()) {
                t = float(i) / float(n);
            } else {
                if (n > 1) t = float(i) / float(n - 1);
                else t = 0.0f;
            }
            // Determine if pixel is lit according to mask
            bool lit = true;
            if (maskTrack.active) {
                float start = maskTrack.start;
                float amount = maskTrack.amount;
                // ring wraps; linear clamps
                if (_layout.wraps()) {
                    // check if t is within [start, start+amount) modulo 1
                    float end = start + amount;
                    float tt = t - start;
                    if (end <= 1.0f) {
                        lit = (t >= start) && (t < end);
                    } else {
                        // wrapped range
                        lit = (t >= start) || (t < end - 1.0f);
                    }
                } else {
                    // linear: clamp segment to valid range
                    float end = start + amount;
                    if (start < 0.0f) start = 0.0f;
                    if (end > 1.0f) end = 1.0f;
                    lit = (t >= start) && (t < end);
                }
            }
            CRGB out = CRGB::Black;
            if (lit) {
                out = baseColor;
                // apply chase pattern if enabled
                if (motionTrack.active && motionTrack.pattern == ModeType::MOTION_CHASE && motionTrack.segmentPixels > 0) {
                    uint16_t head = _layout.indexFrom01(chasePos01);
                    uint16_t segLen = motionTrack.segmentPixels;
                    // map head and segment onto physical indices
                    // compute distance from this pixel to head along wrap direction
                    int32_t diff = int32_t(i) - int32_t(head);
                    if (_layout.wraps()) {
                        diff = (diff % int32_t(n) + int32_t(n)) % int32_t(n);
                    }
                    if (diff >= 0 && diff < int32_t(segLen)) {
                        // brighten inside segment
                        out = motionTrack.color;
                    }
                }
                // scale brightness
                out.nscale8_video(uint8_t(globalBright * 255));
            }
            _leds[i] = out;
        }
        // Draw overlays on top
        for (uint8_t m = 0; m < overlayTrack.count; ++m) {
            OverlayMarker om = overlayTrack.markers[m];
            // map overlay.pos to pixel index
            uint16_t idx = _layout.indexFrom01(om.pos);
            // draw thickness (only contiguous in physical order)
            for (uint8_t k = 0; k < om.thickness; ++k) {
                uint16_t j = idx;
                if (_layout.wraps()) {
                    j = (idx + k) % n;
                } else {
                    if (idx + k >= n) break;
                    j = idx + k;
                }
                _leds[j] = om.color;
            }
        }
        // Apply notification if active
        if (_notifActive) {
            float progress = 0.0f;
            uint32_t elapsed = nowMs - _activeNotif.startMs;
            switch (_activeNotif.type) {
                case NotifType::FLASH: {
                    uint16_t period = (_activeNotif.param == 0 ? 200 : _activeNotif.param);
                    bool on = ((elapsed / (period / 2)) % 2) == 0;
                    if (on) {
                        if (_activeNotif.mode == NotifMode::OVERRIDE) {
                            fill_solid(_leds, n, _activeNotif.color);
                        } else {
                            for (uint16_t i = 0; i < n; ++i) {
                                _leds[i] += _activeNotif.color;
                            }
                        }
                    }
                } break;
                case NotifType::PULSE: {
                    uint8_t a = sin8(uint8_t((elapsed / 4) & 0xFF));
                    for (uint16_t i = 0; i < n; ++i) {
                        CRGB scaled = _activeNotif.color;
                        scaled.nscale8_video(a);
                        if (_activeNotif.mode == NotifMode::OVERRIDE) {
                            _leds[i] = scaled;
                        } else {
                            _leds[i] += scaled;
                        }
                    }
                } break;
                case NotifType::CHASE: {
                    uint16_t segLen = (_activeNotif.param == 0 ? 3 : _activeNotif.param);
                    if (segLen > n) segLen = n;
                    float period = 1500.0f;
                    float frac = fmodf(float(elapsed % (uint32_t)period), period) / period;
                    uint16_t head = _layout.indexFrom01(frac);
                    for (uint16_t k = 0; k < segLen; ++k) {
                        uint16_t idx;
                        if (_layout.wraps()) {
                            idx = (head + k) % n;
                        } else {
                            idx = head + k;
                            if (idx >= n) break;
                        }
                        if (_activeNotif.mode == NotifMode::OVERRIDE) {
                            _leds[idx] = _activeNotif.color;
                        } else {
                            _leds[idx] += _activeNotif.color;
                        }
                    }
                } break;
            }
        }
    }

private:
    // Map a mode to its track
    static TrackType modeToTrack(ModeType mode) {
        switch (mode) {
            case ModeType::COLOR_STATE_PALETTE:
            case ModeType::COLOR_BINARY:
            case ModeType::COLOR_VALUE_GRADIENT:
            case ModeType::COLOR_VALUE_HUE:
            case ModeType::COLOR_CATEGORY_PALETTE:
                return TrackType::COLOR;
            case ModeType::BRIGHTNESS_VALUE:
            case ModeType::BRIGHTNESS_BINARY:
            case ModeType::BRIGHTNESS_GAMMA:
            case ModeType::BRIGHTNESS_LIMITER:
                return TrackType::BRIGHTNESS;
            case ModeType::MASK_FILL:
            case ModeType::MASK_CENTER_FILL:
            case ModeType::MASK_WINDOW_POSITION:
            case ModeType::MASK_TICK_COUNT:
            case ModeType::MASK_SEGMENT_ENABLE:
            case ModeType::MASK_DENSITY:
                return TrackType::MASK;
            case ModeType::MOTION_SOLID:
            case ModeType::MOTION_PULSE:
            case ModeType::MOTION_BLINK:
            case ModeType::MOTION_CHASE:
            case ModeType::MOTION_SCANNER:
            case ModeType::MOTION_TWINKLE:
            case ModeType::MOTION_SPEED:
                return TrackType::MOTION;
            case ModeType::OVERLAY_MARKER_SINGLE:
            case ModeType::OVERLAY_MARKER_THICK:
            case ModeType::OVERLAY_THRESHOLD_MARKS:
            case ModeType::OVERLAY_CLOCK_HANDS:
            case ModeType::OVERLAY_CARDINAL_TICKS:
                return TrackType::OVERLAY;
            default:
                return TrackType::COLOR;
        }
    }
    // Determine if a track is exclusive.  Color, mask and motion are
    // exclusive; brightness and overlay are combinable.
    static bool isExclusiveTrack(TrackType track) {
        return (track == TrackType::COLOR || track == TrackType::MASK || track == TrackType::MOTION);
    }

    CRGB* _leds;
    Layout& _layout;
    LayerConfig _layers[MAX_LAYERS];
    uint8_t _layerCount = 0;
    // Track highest priority used to resolve exclusive tracks during tick
    int _maxColorPri = -32768;
    int _maxMaskPri = -32768;
    int _maxMotionPri = -32768;
    // Notification state
    Notification _activeNotif;
    bool _notifActive = false;
    Notification _notifQueue[MAX_NOTIFS];
    uint8_t _notifQueueCount = 0;
    uint32_t _now = 0;
};

} // namespace LedLayer
