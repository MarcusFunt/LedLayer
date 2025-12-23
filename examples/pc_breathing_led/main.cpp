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
    // Create a Display object with two layers (one for color, one for motion)
    LedLayer::Display<2> display(renderer, layout);

    // -- Layer Data Sources --
    // A constant value to drive the motion. For MOTION_PULSE, the source is not
    // actually used to vary the speed in this simple configuration, but a source
    // is still required. A value of 0.5f provides a moderate, steady pulse.
    float motionValue = 0.5f;
    // A dummy source for the solid color layer.
    static float dummySource = 1.0f;

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
    // This layer creates the gentle pulsing or "breathing" effect.
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

    // -- Simulation Loop --
    // Run the simulation for 100 ticks to show the effect.
    std::cout << "Running Breathing LED Simulation..." << std::endl;
    for (int i = 0; i < 100; ++i) {
        // Provide a simulated time to the display's tick function.
        // Multiplying by 50 slows down the pulse to a reasonable speed.
        display.tick(i * 50);

        // Get the state of the simulated LEDs
        const auto& leds = renderer.getLeds();

        // Print the color of the first LED to show the pulsing brightness.
        // All LEDs will have the same color in this effect.
        std::cout << "Tick " << i << ": ";
        std::cout << "(" << int(leds[0].r) << "," << int(leds[0].g) << "," << int(leds[0].b) << ")" << std::endl;
    }

    return 0;
}
