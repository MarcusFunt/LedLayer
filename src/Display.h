#pragma once

#include <FastLED.h>
#include "Layout.h"
#include "Layer.h"
#include "Notification.h"
#include "Tracks.h"

namespace LedLayer {

template<uint8_t MAX_LAYERS = 8, uint8_t MAX_NOTIFS = 4>
class Display {
public:
    Display(CRGB* leds, Layout& layout);

    bool addLayer(const LayerConfig& cfg);

    bool begin();

    bool notify(const Notification& notif);

    void tick(uint32_t nowMs);

private:
    static TrackType modeToTrack(ModeType mode);
    static bool isExclusiveTrack(TrackType track);

    CRGB* _leds;
    Layout& _layout;
    LayerConfig _layers[MAX_LAYERS];
    uint8_t _layerCount = 0;
    int _maxColorPri = -32768;
    int _maxMaskPri = -32768;
    int _maxMotionPri = -32768;
    Notification _activeNotif;
    bool _notifActive = false;
    Notification _notifQueue[MAX_NOTIFS];
    uint8_t _notifQueueCount = 0;
    uint32_t _now = 0;
};

}
