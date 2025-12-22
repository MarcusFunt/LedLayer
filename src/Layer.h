#pragma once

#include <FastLED.h>
#include "Mode.h"

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
        float start = 0.0f;
    } mask;
    struct MotionParam {
        uint8_t segmentPixels = 3;
        CRGB color = CRGB::White;
        float speed = 1.0f;
    } motion;
    struct OverlayParam {
        float pos = 0.0f;
        CRGB color = CRGB::White;
        uint8_t thickness = 1;
    } overlay;

    int priority = 0;
};

}
