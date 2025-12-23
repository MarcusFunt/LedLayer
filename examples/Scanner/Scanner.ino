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
// LedLayer display object with three layers
LedLayer::Display<3> display(renderer, layout);

// -- Layer Data Sources --
// This value will control the position of the scanner "eye". It will be
// animated in the loop() function to move back and forth.
float scannerPosition = 0.0f;

void setup() {
    // Initialize the FastLED renderer
    renderer.begin();

    // -- Background Layer Configuration --
    // This layer provides a solid, dim red background for the scanner
    // to move across.
    LedLayer::LayerConfig backgroundLayer;

    // A dummy source is required, but the value is not used for a solid color.
    static float dummySource = 1.0f;
    backgroundLayer.source = &dummySource;

    // COLOR_BINARY with a value >= 0.5f will select the `to` color.
    backgroundLayer.mode = LedLayer::ModeType::COLOR_BINARY;
    backgroundLayer.gradient.to = {30, 0, 0}; // Dim red

    // Add the background layer to the display.
    display.addLayer(backgroundLayer);

    // -- Scanner Layer Configuration --
    // This layer creates the moving "eye" of the scanner.
    LedLayer::LayerConfig scannerLayer;

    // Set the data source to our animated scannerPosition variable.
    scannerLayer.source = &scannerPosition;

    // Use the MASK_WINDOW_POSITION mode. This mode creates a "window" or
    // a lit segment on the LED strip. The source value controls the start
    // position of this window.
    scannerLayer.mode = LedLayer::ModeType::MASK_WINDOW_POSITION;

    // The size of the scanner window (e.g., 5 pixels wide). The mask's
    // `amount` is specified as a fraction of the total strip length.
    scannerLayer.mask.amount = 5.0f / NUM_LEDS;

    // The scanner layer has a higher priority than the background, so its
    // color will be shown where the mask is active.
    scannerLayer.priority = 10;

    // We also need a color for the scanner eye itself. This is done with
    // a MOTION_SOLID layer, which is blended on top of the base color.
    LedLayer::LayerConfig scannerMotionLayer;
    scannerMotionLayer.source = &dummySource;
    scannerMotionLayer.mode = LedLayer::ModeType::MOTION_SOLID;
    scannerMotionLayer.motion.color = {255, 0, 0}; // Bright red
    scannerMotionLayer.priority = 10; // Must have same or higher priority

    // Add the scanner layers to the display.
    display.addLayer(scannerLayer);
    display.addLayer(scannerMotionLayer);

    // Initialize the display and all its layers.
    display.begin();
}

void loop() {
    // Animate the scannerPosition value. A sine wave is used to create
    // a smooth back-and-forth motion. The output of sin() is from -1 to 1,
    // so we map it to the 0 to 1 range required by the MASK_WINDOW_POSITION.
    scannerPosition = (sin(millis() / 1000.0f) + 1.0f) / 2.0f;

    // Update the display on each frame.
    display.tick(millis());

    // A small delay to control the frame rate.
    delay(10);
}
