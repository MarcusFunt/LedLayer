#pragma once

#include <FastLED.h>
#include "Mode.h"

namespace LedLayer {

enum class TrackType : uint8_t {
    COLOR,
    BRIGHTNESS,
    MASK,
    MOTION,
    OVERLAY
};

struct ColorTrack {
    bool active = false;
    CRGB color = CRGB::Black;
};

struct BrightnessTrack {
    bool active = false;
    float scale = 1.0f;
    float limit = 1.0f;
};

enum class FillMode : uint8_t {
    NORMAL,
    CENTER
};

struct MaskTrack {
    bool active = false;
    float start = 0.0f;
    float amount = 1.0f;
    FillMode fillMode = FillMode::NORMAL;
};

struct MotionTrack {
    bool active = false;
    ModeType pattern = ModeType::MOTION_SOLID;
    uint8_t segmentPixels = 1;
    CRGB color = CRGB::White;
    float speed = 1.0f;
};

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

}
