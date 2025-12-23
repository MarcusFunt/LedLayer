#pragma once

#include "Renderer.h"
#include <vector>

namespace LedLayer {

class PCRenderer : public Renderer {
public:
    PCRenderer(int numLeds) : _leds(numLeds) {}

    void begin() override {}

    RGB getPixel(int index) const override {
        if (index >= 0 && index < _leds.size()) {
            return _leds[index];
        }
        return {0, 0, 0};
    }

    void setPixel(int index, const RGB& color) override {
        if (index >= 0 && index < _leds.size()) {
            _leds[index] = color;
        }
    }

    void show() override {}

    const std::vector<RGB>& getLeds() const {
        return _leds;
    }

private:
    std::vector<RGB> _leds;
};

}
