#pragma once

#include <stdint.h>

namespace LedLayer {

struct RGB {
    uint8_t r, g, b;
};

class Renderer {
public:
    virtual void begin() = 0;
    virtual RGB getPixel(int index) const = 0;
    virtual void setPixel(int index, const RGB& color) = 0;
    virtual void show() = 0;
};

}
