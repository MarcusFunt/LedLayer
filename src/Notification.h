#pragma once

#include "Renderer.h"

namespace LedLayer {

enum class NotifType : uint8_t {
    FLASH,
    PULSE,
    CHASE
};

enum class NotifMode : uint8_t {
    OVERRIDE,
    OVERLAY
};

struct Notification {
    NotifType type = NotifType::FLASH;
    NotifMode mode = NotifMode::OVERRIDE;
    RGB color = {255, 255, 255};
    uint32_t startMs = 0;
    uint32_t durationMs = 500;
    uint8_t priority = 0;
    uint16_t param = 200;
};

}
