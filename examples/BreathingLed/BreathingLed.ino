#include <FastLED.h>
#include <LedLayer.h>
#include <FastLEDRenderer.h>

// -- Hardware Configuration --
// Number of LEDs in the strip
#define NUM_LEDS 60
// Data pin for the LED strip
#define LED_PIN 6

// -- Global Objects --
// FastLED array for LED data
CRGB leds[NUM_LEDS];
// LedLayer layout object (a simple linear strip)
LedLayer::LinearLayout layout(NUM_LEDS);
// LedLayer renderer for FastLED
LedLayer::FastLEDRenderer<NEOPIXEL, LED_PIN, GRB> renderer(leds, NUM_LEDS);
// LedLayer display object with two layers (one for color, one for motion)
LedLayer::Display<2> display(renderer, layout);

// -- Layer Data Sources --
// A constant value to drive the motion. For MOTION_PULSE, the source is not
// actually used to vary the speed in this simple configuration, but a source
// is still required. A value of 0.5f provides a moderate, steady pulse.
float motionValue = 0.5f;
// A dummy source for the solid color layer.
static float dummySource = 1.0f;

void setup() {
    // Initialize the FastLED renderer
    renderer.begin();

    // -- Base Color Layer --
    // This layer provides the base color that the breathing layer will modulate.
    LedLayer::LayerConfig colorLayer;
    colorLayer.source = &dummySource;
    // COLOR_BINARY with a value >= 0.5f selects the `to` color.
    colorLayer.mode = LedLayer::ModeType::COLOR_BINARY;
    // Set the color to a warm white.
    colorLayer.gradient.to = {255, 160, 40};
    display.addLayer(colorLayer);

    // -- Breathing Layer Configuration --
    // This layer creates a gentle pulsing or "breathing" effect.
    LedLayer::LayerConfig breathingLayer;

    // Set the data source for the layer.
    breathingLayer.source = &motionValue;

    // Use the MOTION_PULSE mode. This mode creates a sine-wave brightness
    // modulation over the base color provided by the colorLayer.
    breathingLayer.mode = LedLayer::ModeType::MOTION_PULSE;

    // Add the configured layer to the display.
    display.addLayer(breathingLayer);

    // Initialize the display and all its layers.
    display.begin();
}

void loop() {
    // Update the display on each frame. `millis()` is used to provide
    // a time source for the animation.
    display.tick(millis());

    // A small delay to control the frame rate.
    delay(10);
}
