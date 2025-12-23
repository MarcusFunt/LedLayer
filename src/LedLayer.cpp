#include <algorithm>
#include "Display.h"
#include "Layout.h"
#include "Color.h"
#include <cmath>

namespace LedLayer {

template<uint8_t MAX_LAYERS, uint8_t MAX_NOTIFS>
Display<MAX_LAYERS, MAX_NOTIFS>::Display(Renderer& renderer, Layout& layout)
    : _renderer(renderer), _layout(layout) {}

template<uint8_t MAX_LAYERS, uint8_t MAX_NOTIFS>
bool Display<MAX_LAYERS, MAX_NOTIFS>::addLayer(const LayerConfig& cfg) {
    if (_layerCount >= MAX_LAYERS) return false;
    _layers[_layerCount++] = cfg;
    return true;
}

template<uint8_t MAX_LAYERS, uint8_t MAX_NOTIFS>
bool Display<MAX_LAYERS, MAX_NOTIFS>::begin() {
    // No strict priority checking at begin(). The highest-priority layer
    // processed during tick() will win.
    for (uint8_t i = 0; i < _layerCount; ++i) {
        _layers[i].hystState = 0.0f;
    }
    return true;
}

template<uint8_t MAX_LAYERS, uint8_t MAX_NOTIFS>
bool Display<MAX_LAYERS, MAX_NOTIFS>::notify(const Notification& notif) {
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

template<uint8_t MAX_LAYERS, uint8_t MAX_NOTIFS>
void Display<MAX_LAYERS, MAX_NOTIFS>::tick(uint32_t nowMs) {
    _now = nowMs;

    if (_notifActive) {
        uint32_t elapsed = nowMs - _activeNotif.startMs;
        if (elapsed >= _activeNotif.durationMs) {
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

    ColorTrack colorTrack;
    BrightnessTrack brightnessTrack;
    MaskTrack maskTrack;
    MotionTrack motionTrack;
    OverlayTrack overlayTrack;

    _maxColorPri = -32768;
    _maxMaskPri = -32768;
    _maxMotionPri = -32768;

    for (uint8_t i = 0; i < _layerCount; ++i) {
        const LayerConfig& cfg = _layers[i];
        if (!cfg.source) continue;
        float raw = *cfg.source;
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

        float val = mapped;
        if (cfg.emaEnabled) {
            if (!cfg.emaInitialized) {
                const_cast<LayerConfig&>(cfg).emaState = val;
                const_cast<LayerConfig&>(cfg).emaInitialized = true;
            } else {
                float state = cfg.emaState;
                state = state + cfg.emaAlpha * (val - state);
                const_cast<LayerConfig&>(cfg).emaState = state;
                val = state;
            }
        }

        float discVal = val;
        if (cfg.hystEnabled) {
            float prev = cfg.hystState;
            float half = cfg.hystBand;
            if (fabsf(val - prev) <= half) {
                discVal = prev;
            } else {
                discVal = (val > prev) ? 1.0f : 0.0f;
                const_cast<LayerConfig&>(cfg).hystState = discVal;
            }
        }

        TrackType track = modeToTrack(cfg.mode);
        switch (track) {
            case TrackType::COLOR: {
                if (!colorTrack.active || cfg.priority >= _maxColorPri) {
                    RGB c = {0, 0, 0};
                    switch (cfg.mode) {
                        case ModeType::COLOR_STATE_PALETTE:
                        case ModeType::COLOR_CATEGORY_PALETTE: {
                            uint8_t idx = uint8_t(discVal + 0.5f);
                            if (idx < cfg.palette.count) {
                                c = cfg.palette.colors[idx];
                            } else if (cfg.palette.count > 0) {
                                c = cfg.palette.colors[cfg.palette.count - 1];
                            }
                        } break;
                        case ModeType::COLOR_VALUE_GRADIENT: {
                            // This mode is handled per-pixel. Defer color calculation.
                        } break;
                        case ModeType::COLOR_VALUE_HUE: {
                            uint8_t h = uint8_t(val * 255.0f);
                            c = hsvToRgb(h, 255, 255);
                        } break;
                        case ModeType::COLOR_BINARY: {
                            c = val >= 0.5f ? cfg.gradient.to : cfg.gradient.from;
                        } break;
                        default:
                            break;
                    }
                    colorTrack.active = true;
                    colorTrack.color = c;
                    colorTrack.layer = &cfg;
                    colorTrack.value = val;
                    _maxColorPri = cfg.priority;
                }
            } break;
            case TrackType::BRIGHTNESS: {
                float scale = 1.0f;
                switch (cfg.mode) {
                    case ModeType::BRIGHTNESS_VALUE: {
                        scale = val;
                    } break;
                    case ModeType::BRIGHTNESS_BINARY: {
                        scale = val >= 0.5f ? 1.0f : 0.0f;
                    } break;
                    case ModeType::BRIGHTNESS_GAMMA: {
                        float g = cfg.brightness.gamma;
                        scale = powf(val, g);
                    } break;
                    case ModeType::BRIGHTNESS_LIMITER: {
                        brightnessTrack.limit = fminf(brightnessTrack.limit, val);
                    } break;
                    default:
                        break;
                }
                brightnessTrack.active = true;
                brightnessTrack.scale *= scale;
            } break;
            case TrackType::MASK: {
                if (!maskTrack.active || cfg.priority >= _maxMaskPri) {
                    float start, amount;
                    if (cfg.mode == ModeType::MASK_WINDOW_POSITION) {
                        start = val;
                        amount = cfg.mask.amount;
                    } else {
                        start = cfg.mask.start;
                        amount = val;
                    }

                    if (amount < 0.0f) amount = 0.0f;
                    if (amount > 1.0f) amount = 1.0f;

                    maskTrack.active = true;
                    maskTrack.start = start;
                    maskTrack.amount = amount;
                    if (cfg.mode == ModeType::MASK_CENTER_FILL) {
                        maskTrack.fillMode = FillMode::CENTER;
                    } else {
                        maskTrack.fillMode = FillMode::NORMAL;
                    }
                    _maxMaskPri = cfg.priority;
                }
            } break;
            case TrackType::MOTION: {
                if (!motionTrack.active || cfg.priority >= _maxMotionPri) {
                    motionTrack.pattern = cfg.mode;
                    motionTrack.segmentPixels = cfg.motion.segmentPixels;
                    motionTrack.color = cfg.motion.color;
                    motionTrack.speed = cfg.motion.speed * (0.2f + val * 2.0f);
                    motionTrack.active = true;
                    _maxMotionPri = cfg.priority;
                }
            } break;
            case TrackType::OVERLAY: {
                if (overlayTrack.count < MAX_OVERLAYS) {
                    OverlayMarker marker;
                    marker.pos = cfg.overlay.pos;
                    marker.color = cfg.overlay.color;
                    marker.thickness = cfg.overlay.thickness;
                    overlayTrack.markers[overlayTrack.count++] = marker;
                }
            } break;
        }
    }

    uint16_t n = _layout.size();
    RGB baseColor = colorTrack.active ? colorTrack.color : RGB{0, 0, 0};
    float globalBright = brightnessTrack.active ? brightnessTrack.scale : 1.0f;
    if (globalBright < 0.0f) globalBright = 0.0f;
    if (globalBright > brightnessTrack.limit) globalBright = brightnessTrack.limit;
    float chasePos01 = 0.0f;
    if (motionTrack.active && motionTrack.pattern == ModeType::MOTION_CHASE && n > 0) {
        float period = 2000.0f / motionTrack.speed;
        float frac = fmodf(float(nowMs % (uint32_t)period), period) / period;
        chasePos01 = frac;
    }

    for (uint16_t i = 0; i < n; ++i) {
        float t;
        if (_layout.wraps()) {
            t = float(i) / float(n);
        } else {
            if (n > 1) t = float(i) / float(n - 1);
            else t = 0.0f;
        }

        bool lit = true;
        if (maskTrack.active) {
            if (maskTrack.fillMode == FillMode::CENTER) {
                float halfAmount = maskTrack.amount / 2.0f;
                float start = 0.5f - halfAmount;
                float end = 0.5f + halfAmount;
                lit = (t >= start) && (t < end);
            } else {
                float start = maskTrack.start;
                float amount = maskTrack.amount;
                if (_layout.wraps()) {
                    float end = start + amount;
                    if (end <= 1.0f) {
                        lit = (t >= start) && (t < end);
                    } else {
                        lit = (t >= start) || (t < end - 1.0f);
                    }
                } else {
                    float end = start + amount;
                    if (start < 0.0f) start = 0.0f;
                    if (end > 1.0f) end = 1.0f;
                    lit = (t >= start) && (t < end);
                }
            }
        }

        RGB out = {0, 0, 0};
        if (lit) {
            RGB pixelColor = baseColor;
            if (colorTrack.active && colorTrack.layer && colorTrack.layer->mode == ModeType::COLOR_VALUE_GRADIENT) {
                const auto& cfg = *colorTrack.layer;
                float val = colorTrack.value;
                if (val > 0 && t <= val) {
                    float t_grad = t / val;
                    uint8_t r = uint8_t((1.0f - t_grad) * cfg.gradient.from.r + t_grad * cfg.gradient.to.r);
                    uint8_t g = uint8_t((1.0f - t_grad) * cfg.gradient.from.g + t_grad * cfg.gradient.to.g);
                    uint8_t b = uint8_t((1.0f - t_grad) * cfg.gradient.from.b + t_grad * cfg.gradient.to.b);
                    pixelColor = {r, g, b};
                }
            }
            out = pixelColor;

            if (motionTrack.active) {
                switch (motionTrack.pattern) {
                    case ModeType::MOTION_SOLID: {
                        out.r = std::max(out.r, motionTrack.color.r);
                        out.g = std::max(out.g, motionTrack.color.g);
                        out.b = std::max(out.b, motionTrack.color.b);
                    } break;
                    case ModeType::MOTION_PULSE: {
                        float pulseBrightness = (sinf(_now / 256.0f) + 1.0f) / 2.0f;
                        out.r *= pulseBrightness;
                        out.g *= pulseBrightness;
                        out.b *= pulseBrightness;
                    } break;
                    case ModeType::MOTION_CHASE: {
                        if (motionTrack.segmentPixels > 0) {
                            uint16_t head = _layout.indexFrom01(chasePos01);
                            uint16_t segLen = motionTrack.segmentPixels;
                            int32_t diff = int32_t(i) - int32_t(head);
                            if (_layout.wraps()) {
                                diff = (diff % int32_t(n) + int32_t(n)) % int32_t(n);
                            }
                            if (diff >= 0 && diff < int32_t(segLen)) {
                                out = motionTrack.color;
                            }
                        }
                    } break;
                    default:
                        break;
                }
            }
            out.r *= globalBright;
            out.g *= globalBright;
            out.b *= globalBright;
        }
        _renderer.setPixel(i, out);
    }

    for (uint8_t m = 0; m < overlayTrack.count; ++m) {
        OverlayMarker om = overlayTrack.markers[m];
        uint16_t idx = _layout.indexFrom01(om.pos);
        for (uint8_t k = 0; k < om.thickness; ++k) {
            uint16_t j = idx;
            if (_layout.wraps()) {
                j = (idx + k) % n;
            } else {
                if (idx + k >= n) break;
                j = idx + k;
            }
            _renderer.setPixel(j, om.color);
        }
    }

    if (_notifActive) {
        uint32_t elapsed = nowMs - _activeNotif.startMs;
        switch (_activeNotif.type) {
            case NotifType::FLASH: {
                uint16_t period = (_activeNotif.param == 0 ? 200 : _activeNotif.param);
                bool on = ((elapsed / (period / 2)) % 2) == 0;
                if (on) {
                    for (uint16_t i = 0; i < n; ++i) {
                        if (_activeNotif.mode == NotifMode::OVERRIDE) {
                            _renderer.setPixel(i, _activeNotif.color);
                        } else {
                            RGB p = _renderer.getPixel(i);
                            p.r = std::min(255, int(p.r) + _activeNotif.color.r);
                            p.g = std::min(255, int(p.g) + _activeNotif.color.g);
                            p.b = std::min(255, int(p.b) + _activeNotif.color.b);
                            _renderer.setPixel(i, p);
                        }
                    }
                }
            } break;
            case NotifType::PULSE: {
                float a = (sinf(elapsed / 256.0f) + 1.0f) / 2.0f;
                for (uint16_t i = 0; i < n; ++i) {
                    RGB scaled = _activeNotif.color;
                    scaled.r *= a;
                    scaled.g *= a;
                    scaled.b *= a;
                    if (_activeNotif.mode == NotifMode::OVERRIDE) {
                        _renderer.setPixel(i, scaled);
                    } else {
                        RGB p = _renderer.getPixel(i);
                        p.r = std::min(255, int(p.r) + scaled.r);
                        p.g = std::min(255, int(p.g) + scaled.g);
                        p.b = std::min(255, int(p.b) + scaled.b);
                        _renderer.setPixel(i, p);
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
                        _renderer.setPixel(idx, _activeNotif.color);
                    } else {
                        RGB p = _renderer.getPixel(idx);
                        p.r = std::min(255, int(p.r) + _activeNotif.color.r);
                        p.g = std::min(255, int(p.g) + _activeNotif.color.g);
                        p.b = std::min(255, int(p.b) + _activeNotif.color.b);
                        _renderer.setPixel(idx, p);
                    }
                }
            } break;
        }
    }
    _renderer.show();
}

template<uint8_t MAX_LAYERS, uint8_t MAX_NOTIFS>
TrackType Display<MAX_LAYERS, MAX_NOTIFS>::modeToTrack(ModeType mode) {
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

template<uint8_t MAX_LAYERS, uint8_t MAX_NOTIFS>
bool Display<MAX_LAYERS, MAX_NOTIFS>::isExclusiveTrack(TrackType track) {
    return (track == TrackType::COLOR || track == TrackType::MASK || track == TrackType::MOTION);
}

template class Display<1, 4>;
template class Display<2, 4>;
template class Display<3, 4>;
template class Display<4, 4>;
template class Display<5, 4>;
template class Display<8, 4>;

}
