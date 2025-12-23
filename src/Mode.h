#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <cstdint>
#endif

namespace LedLayer {

enum class ModeType : uint8_t {
    // Color modes (COLOR track, exclusive)
    COLOR_STATE_PALETTE,
    COLOR_BINARY,
    COLOR_VALUE_GRADIENT,
    COLOR_VALUE_HUE,
    COLOR_CATEGORY_PALETTE,

    // Brightness modes (BRIGHTNESS track, combinable)
    BRIGHTNESS_VALUE,
    BRIGHTNESS_BINARY,
    BRIGHTNESS_GAMMA,
    BRIGHTNESS_LIMITER,

    // Mask modes (MASK track, exclusive)
    MASK_FILL,
    MASK_CENTER_FILL,
    MASK_WINDOW_POSITION,
    MASK_TICK_COUNT,
    MASK_SEGMENT_ENABLE,
    MASK_DENSITY,

    // Motion modes (MOTION track, exclusive pattern + optional speed)
    MOTION_SOLID,
    MOTION_PULSE,
    MOTION_BLINK,
    MOTION_CHASE,
    MOTION_SCANNER,
    MOTION_TWINKLE,
    MOTION_SPEED,

    // Overlay modes (OVERLAY track, combinable)
    OVERLAY_MARKER_SINGLE,
    OVERLAY_MARKER_THICK,
    OVERLAY_THRESHOLD_MARKS,
    OVERLAY_CLOCK_HANDS,
    OVERLAY_CARDINAL_TICKS
};

}
