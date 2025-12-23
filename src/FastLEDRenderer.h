#pragma once

#if defined(ARDUINO)

#include <FastLED.h>
#include "Renderer.h"

namespace LedLayer {

template<ESPIChipsets CHIPSET, uint8_t DATA_PIN, EOrder COLOR_ORDER>
class FastLEDRenderer : public Renderer {
public:
    FastLEDRenderer(CRGB* leds, int numLeds) : _leds(leds), _numLeds(numLeds) {}

    void begin() override {
        FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(_leds, _numLeds);
    }

    RGB getPixel(int index) const override {
        if (index >= 0 && index < _numLeds) {
            const CRGB& p = _leds[index];
            return {p.r, p.g, p.b};
        }
        return {0, 0, 0};
    }

    void setPixel(int index, const RGB& color) override {
        if (index >= 0 && index < _numLeds) {
            _leds[index] = CRGB(color.r, color.g, color.b);
        }
    }

    void show() override {
        FastLED.show();
    }

private:
    CRGB* _leds;
    int _numLeds;
};

}

#endif
