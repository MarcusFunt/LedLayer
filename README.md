# LedLayer: A Layered LED Display Library for Arduino

LedLayer is an Arduino library for composing multiple pieces of information on a single addressable LED strip or ring.

## Vision

- **Layer → Mode → Track pipeline:** Inputs (sensors/variables) feed layers, layers choose modes, modes write to dedicated tracks (color, brightness, mask, motion, overlay), and a layout-aware renderer maps tracks to LEDs.
- **Layout abstraction:** Linear strips and circular rings are first-class targets; additional shapes can be added by implementing new layouts.
- **Conflict-aware composition:** Exclusive tracks (e.g., color, mask, motion pattern) resolve via priority, while combinable tracks (brightness, overlays) merge contributions.
- **Notifications:** Temporary overrides (flash, pulse, chase) can supersede the normal composition without altering layer state.

For full conceptual details, see [docs/architecture.md](docs/architecture.md).

## Installation

1.  **Download the library:** Download the latest release from the [GitHub repository](https://github.com/MarcusFunt/LedLayer).
2.  **Install in the Arduino IDE:** In the Arduino IDE, go to `Sketch > Include Library > Add .ZIP Library...` and select the downloaded ZIP file.

## Dependencies

This library depends on the [FastLED](https://github.com/FastLED/FastLED) library. Please ensure it is installed before using LedLayer.

## Basic Usage

Here's a simple example of how to use LedLayer to display a sensor value on an LED strip:

```cpp
#include <FastLED.h>
#include <LedLayer.h>

#define NUM_LEDS 60
#define LED_PIN 6

CRGB leds[NUM_LEDS];
LedLayer::LinearLayout layout(NUM_LEDS);
LedLayer::Display<1> display(leds, layout);

float sensorValue = 0.5f;

void setup() {
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    LedLayer::LayerConfig gaugeLayer;
    gaugeLayer.source = &sensorValue;
    gaugeLayer.mode = LedLayer::ModeType::COLOR_VALUE_GRADIENT;
    gaugeLayer.gradient.from = CRGB::Green;
    gaugeLayer.gradient.to = CRGB::Red;
    display.addLayer(gaugeLayer);

    display.begin();
}

void loop() {
    // Update the sensor value (e.g., from a sensor)
    sensorValue = (sin(millis() / 2000.0f) + 1.0f) / 2.0f;

    display.tick(millis());
    FastLED.show();
    delay(10);
}
```

## Examples

For more detailed examples, please see the `examples` directory.

## License

This library is released under the MIT License. See the `LICENSE` file for more details.
