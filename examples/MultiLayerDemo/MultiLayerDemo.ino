#include <FastLED.h>
#include <LedLayer.h>
#include <FastLEDRenderer.h>

#define NUM_LEDS 60
#define LED_PIN 6

CRGB leds[NUM_LEDS];
LedLayer::LinearLayout layout(NUM_LEDS);
LedLayer::FastLEDRenderer<NEOPIXEL, LED_PIN, GRB> renderer(leds, NUM_LEDS);
LedLayer::Display<4> display(renderer, layout);

float discreteState = 0.0f;
float continuousValue = 0.5f;
float levelBar = 0.75f;
float motionSpeed = 0.5f;

void setup() {
    renderer.begin();

    LedLayer::LayerConfig colorLayer;
    colorLayer.source = &discreteState;
    colorLayer.mode = LedLayer::ModeType::COLOR_STATE_PALETTE;
    colorLayer.palette.count = 3;
    colorLayer.palette.colors[0] = {255, 0, 0};
    colorLayer.palette.colors[1] = {0, 255, 0};
    colorLayer.palette.colors[2] = {0, 0, 255};
    display.addLayer(colorLayer);

    LedLayer::LayerConfig brightnessLayer;
    brightnessLayer.source = &continuousValue;
    brightnessLayer.mode = LedLayer::ModeType::BRIGHTNESS_VALUE;
    display.addLayer(brightnessLayer);

    LedLayer::LayerConfig maskLayer;
    maskLayer.source = &levelBar;
    maskLayer.mode = LedLayer::ModeType::MASK_FILL;
    display.addLayer(maskLayer);

    LedLayer::LayerConfig motionLayer;
    motionLayer.source = &motionSpeed;
    motionLayer.mode = LedLayer::ModeType::MOTION_CHASE;
    motionLayer.motion.color = {255, 255, 255};
    motionLayer.motion.segmentPixels = 5;
    display.addLayer(motionLayer);

    display.begin();
}

void loop() {
    discreteState = (int)((millis() / 2000) % 3);
    continuousValue = (sin(millis() / 3000.0f) + 1.0f) / 2.0f;
    levelBar = (sin(millis() / 1000.0f) + 1.0f) / 2.0f;
    motionSpeed = (sin(millis() / 4000.0f) + 1.0f) / 2.0f;

    display.tick(millis());
    delay(10);
}
