#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <cstdint>
#endif

namespace LedLayer {

class Layout {
public:
    virtual ~Layout() = default;
    virtual uint16_t size() const = 0;
    virtual bool wraps() const = 0;
    virtual uint16_t indexFrom01(float t) const = 0;
};

class LinearLayout : public Layout {
public:
    explicit LinearLayout(uint16_t count);
    uint16_t size() const override;
    bool wraps() const override;
    uint16_t indexFrom01(float t) const override;
private:
    uint16_t _count;
};

class RingLayout : public Layout {
public:
    RingLayout(uint16_t count, uint16_t offset = 0, bool clockwise = true);
    uint16_t size() const override;
    bool wraps() const override;
    uint16_t indexFrom01(float t) const override;
private:
    uint16_t _count;
    uint16_t _offset;
    bool _clockwise;
};

}
