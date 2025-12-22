#include <FastLED.h>
#include <LedLayer.h>

#define NUM_LEDS 60
#define LED_PIN 6

CRGB leds[NUM_LEDS];
LedLayer::LinearLayout layout(NUM_LEDS);
LedLayer::Display<3> display(leds, layout);

float sensorValue = 0.5f;

void setup() {
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    LedLayer::LayerConfig gaugeLayer;
    gaugeLayer.source = &sensorValue;
    gaugeLayer.mode = LedLayer::ModeType::COLOR_VALUE_GRADIENT;
    gaugeLayer.gradient.from = CRGB::Green;
    gaugeLayer.gradient.to = CRGB::Red;
    display.addLayer(gaugeLayer);

    LedLayer::LayerConfig brightnessLayer;
    brightnessLayer.source = &sensorValue;
    brightnessLayer.mode = LedLayer::ModeType::BRIGHTNESS_VALUE;
    display.addLayer(brightnessLayer);

    LedLayer::LayerConfig markerLayer;
    markerLayer.mode = LedLayer::ModeType::OVERLAY_MARKER_SINGLE;
    markerLayer.overlay.pos = 0.75f;
    markerLayer.overlay.color = CRGB::Blue;
    display.addLayer(markerLayer);

    display.begin();
}

void loop() {
    sensorValue = (sin(millis() / 2000.0f) + 1.0f) / 2.0f;

    display.tick(millis());
    FastLED.show();
    delay(10);
}
