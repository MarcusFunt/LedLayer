#pragma once

#include "Layer.h"
#include "Mode.h"
#include "Renderer.h"

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
    RGB color = {0, 0, 0};
    const LayerConfig* layer = nullptr;
    float value = 0.0f;
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
    RGB color = {255, 255, 255};
    float speed = 1.0f;
};

static const uint8_t MAX_OVERLAYS = 8;
struct OverlayMarker {
    float pos = 0.0f;
    RGB color = {255, 255, 255};
    uint8_t thickness = 1;
};

struct OverlayTrack {
    uint8_t count = 0;
    OverlayMarker markers[MAX_OVERLAYS];
};

}
