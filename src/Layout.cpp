#include "Layout.h"

namespace LedLayer {

LinearLayout::LinearLayout(uint16_t count) : _count(count) {}

uint16_t LinearLayout::size() const {
    return _count;
}

bool LinearLayout::wraps() const {
    return false;
}

uint16_t LinearLayout::indexFrom01(float t) const {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return uint16_t(t * (_count - 1));
}

RingLayout::RingLayout(uint16_t count, uint16_t offset, bool clockwise)
    : _count(count), _offset(offset), _clockwise(clockwise) {}

uint16_t RingLayout::size() const {
    return _count;
}

bool RingLayout::wraps() const {
    return true;
}

uint16_t RingLayout::indexFrom01(float t) const {
    if (_clockwise) {
        return (_offset + uint16_t(t * _count)) % _count;
    } else {
        return (_offset + _count - uint16_t(t * _count)) % _count;
    }
}

}
