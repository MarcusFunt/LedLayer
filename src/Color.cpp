#include "Color.h"
#include <cmath>

namespace LedLayer {

RGB hsvToRgb(uint8_t h, uint8_t s, uint8_t v) {
    RGB out;
    if (s == 0) {
        out.r = out.g = out.b = v;
        return out;
    }

    float region = h / 43.0f;
    uint8_t i = floor(region);
    uint8_t f = (h % 43) * 6;

    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * f) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - f)) >> 8))) >> 8;

    switch (i) {
        case 0: out.r = v; out.g = t; out.b = p; break;
        case 1: out.r = q; out.g = v; out.b = p; break;
        case 2: out.r = p; out.g = v; out.b = t; break;
        case 3: out.r = p; out.g = q; out.b = v; break;
        case 4: out.r = t; out.g = p; out.b = v; break;
        default: out.r = v; out.g = p; out.b = q; break;
    }

    return out;
}

}
