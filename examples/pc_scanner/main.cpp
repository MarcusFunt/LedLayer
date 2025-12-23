#include <iostream>
#include <vector>
#include <cmath>
#include <LedLayer.h>
#include <PCRenderer.h>

// Number of LEDs to simulate
#define NUM_LEDS 60

int main() {
    // -- Simulation Setup --
    // Create a renderer for the PC simulation and a linear layout
    LedLayer::PCRenderer renderer(NUM_LEDS);
    LedLayer::LinearLayout layout(NUM_LEDS);
    // Create a Display object with three layers (background + scanner mask + scanner color)
    LedLayer::Display<3> display(renderer, layout);

    // -- Layer Data Sources --
    // This value will control the position of the scanner "eye".
    float scannerPosition = 0.0f;
    // A dummy source for the solid color layers.
    static float dummySource = 1.0f;

    // -- Background Layer Configuration --
    // A solid, dim red background.
    LedLayer::LayerConfig backgroundLayer;
    backgroundLayer.source = &dummySource;
    backgroundLayer.mode = LedLayer::ModeType::COLOR_BINARY;
    backgroundLayer.gradient.to = {30, 0, 0};
    display.addLayer(backgroundLayer);

    // -- Scanner Layer Configuration --
    // This layer defines the moving mask for the scanner "eye".
    LedLayer::LayerConfig scannerMaskLayer;
    scannerMaskLayer.source = &scannerPosition;
    scannerMaskLayer.mode = LedLayer::ModeType::MASK_WINDOW_POSITION;
    scannerMaskLayer.mask.amount = 5.0f / NUM_LEDS; // 5-pixel wide window
    scannerMaskLayer.priority = 10;
    display.addLayer(scannerMaskLayer);

    // This layer defines the color of the scanner "eye" using a SOLID motion.
    // Motion layers are blended on top of the base color.
    LedLayer::LayerConfig scannerMotionLayer;
    scannerMotionLayer.source = &dummySource;
    scannerMotionLayer.mode = LedLayer::ModeType::MOTION_SOLID;
    scannerMotionLayer.motion.color = {255, 0, 0}; // Bright red
    scannerMotionLayer.priority = 10;
    display.addLayer(scannerMotionLayer);

    // Initialize the display.
    display.begin();

    // -- Simulation Loop --
    std::cout << "Running Scanner LED Simulation..." << std::endl;
    for (int i = 0; i < 100; ++i) {
        // Animate the scanner position using a sine wave for smooth motion.
        scannerPosition = (sin(i / 20.0f) + 1.0f) / 2.0f;

        // Update the display with a simulated time.
        display.tick(i * 50);

        // Get the state of the simulated LEDs
        const auto& leds = renderer.getLeds();

        // Print the colors of the first 10 LEDs to show the effect.
        std::cout << "Tick " << i << ": ";
        for (int j = 0; j < 10; ++j) {
            std::cout << "(" << int(leds[j].r) << "," << int(leds[j].g) << "," << int(leds[j].b) << ") ";
        }
        std::cout << std::endl;
    }

    return 0;
}
