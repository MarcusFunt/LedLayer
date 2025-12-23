#pragma once

#include "Renderer.h"
#include <vector>
#include <iostream>

namespace LedLayer {

class PCRenderer : public Renderer {
public:
    PCRenderer(int numLeds) : _leds(numLeds) {}

    void begin() override {
        // No-op for PC renderer
    }

    void setPixel(int i, RGB color) override {
        if (i >= 0 && i < _leds.size()) {
            _leds[i] = color;
        }
    }

    RGB getPixel(int i) const override {
        if (i >= 0 && i < _leds.size()) {
            return _leds[i];
        }
        return {0, 0, 0};
    }

    void show() override {
        // In a real PC application, this might draw to a window.
        // For these examples, we'll do nothing here and let the
        // main loop inspect the LED state directly.
    }

    const std::vector<RGB>& getLeds() const {
        return _leds;
    }

private:
    std::vector<RGB> _leds;
};

}
