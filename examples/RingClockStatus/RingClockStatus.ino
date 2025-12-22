#include <FastLED.h>
#include <LedLayer.h>

#define NUM_LEDS 60
#define LED_PIN 6

CRGB leds[NUM_LEDS];
LedLayer::RingLayout layout(NUM_LEDS);
LedLayer::Display<5> display(leds, layout);

float statusValue = 0.5f;
float hour = 10.0f;
float minute = 15.0f;
float second = 30.0f;

void setup() {
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    LedLayer::LayerConfig statusLayer;
    statusLayer.source = &statusValue;
    statusLayer.mode = LedLayer::ModeType::MASK_FILL;
    display.addLayer(statusLayer);

    LedLayer::LayerConfig hourHand;
    hourHand.source = &hour;
    hourHand.inMin = 0;
    hourHand.inMax = 12;
    hourHand.wrap = true;
    hourHand.mode = LedLayer::ModeType::OVERLAY_MARKER_THICK;
    hourHand.overlay.color = CRGB::Red;
    hourHand.overlay.thickness = 3;
    display.addLayer(hourHand);

    LedLayer::LayerConfig minuteHand;
    minuteHand.source = &minute;
    minuteHand.inMin = 0;
    minuteHand.inMax = 60;
    minuteHand.wrap = true;
    minuteHand.mode = LedLayer::ModeType::OVERLAY_MARKER_THICK;
    minuteHand.overlay.color = CRGB::Green;
    minuteHand.overlay.thickness = 2;
    display.addLayer(minuteHand);

    LedLayer::LayerConfig secondHand;
    secondHand.source = &second;
    secondHand.inMin = 0;
    secondHand.inMax = 60;
    secondHand.wrap = true;
    secondHand.mode = LedLayer::ModeType::OVERLAY_MARKER_SINGLE;
    secondHand.overlay.color = CRGB::Blue;
    display.addLayer(secondHand);

    display.begin();
}

void loop() {
    statusValue = (sin(millis() / 5000.0f) + 1.0f) / 2.0f;

    // This is a simplified clock for demonstration purposes.
    // In a real application, you would use a real-time clock (RTC)
    // to get the current time.
    second = (millis() / 1000) % 60;
    minute = (millis() / (1000 * 60)) % 60;
    hour = (millis() / (1000 * 60 * 60)) % 12;


    display.tick(millis());
    FastLED.show();
    delay(10);
}
