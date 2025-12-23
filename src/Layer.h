#pragma once

#include "Mode.h"
#include "Renderer.h"

namespace LedLayer {

struct LayerConfig {
    const float* source = nullptr;

    float inMin = 0.0f;
    float inMax = 1.0f;
    bool clamp = true;
    bool wrap = false;

    bool emaEnabled = false;
    float emaAlpha = 0.1f;
    mutable float emaState = 0;
    mutable bool emaInitialized = false;

    bool hystEnabled = false;
    float hystBand = 0.05f;
    mutable float hystState = 0;

    ModeType mode = ModeType::COLOR_STATE_PALETTE;

    struct PaletteParam {
        uint8_t count = 0;
        RGB colors[8];
    } palette;
    struct GradientParam {
        RGB from = {0, 0, 0};
        RGB to   = {255, 255, 255};
    } gradient;
    struct BrightnessParam {
        float gamma = 1.0f;
    } brightness;
    struct MaskParam {
        float start = 0.0f;
    } mask;
    struct MotionParam {
        uint8_t segmentPixels = 3;
        RGB color = {255, 255, 255};
        float speed = 1.0f;
    } motion;
    struct OverlayParam {
        float pos = 0.0f;
        RGB color = {255, 255, 255};
        uint8_t thickness = 1;
    } overlay;

    int priority = 0;
};

}
