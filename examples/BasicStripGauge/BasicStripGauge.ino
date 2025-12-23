#include <FastLED.h>
#include <LedLayer.h>
#include <FastLEDRenderer.h>

#define NUM_LEDS 60
#define LED_PIN 6

CRGB leds[NUM_LEDS];
LedLayer::LinearLayout layout(NUM_LEDS);
LedLayer::FastLEDRenderer<NEOPIXEL, LED_PIN, GRB> renderer(leds, NUM_LEDS);
LedLayer::Display<3> display(renderer, layout);

float sensorValue = 0.5f;

void setup() {
    renderer.begin();

    LedLayer::LayerConfig gaugeLayer;
    gaugeLayer.source = &sensorValue;
    gaugeLayer.mode = LedLayer::ModeType::COLOR_VALUE_GRADIENT;
    gaugeLayer.gradient.from = {0, 255, 0};
    gaugeLayer.gradient.to = {255, 0, 0};
    display.addLayer(gaugeLayer);

    LedLayer::LayerConfig brightnessLayer;
    brightnessLayer.source = &sensorValue;
    brightnessLayer.mode = LedLayer::ModeType::BRIGHTNESS_VALUE;
    display.addLayer(brightnessLayer);

    LedLayer::LayerConfig markerLayer;
    markerLayer.mode = LedLayer::ModeType::OVERLAY_MARKER_SINGLE;
    markerLayer.overlay.pos = 0.75f;
    markerLayer.overlay.color = {0, 0, 255};
    display.addLayer(markerLayer);

    display.begin();
}

void loop() {
    sensorValue = (sin(millis() / 2000.0f) + 1.0f) / 2.0f;

    display.tick(millis());
    delay(10);
}
