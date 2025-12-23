#include <iostream>
#include <vector>
#include <cmath>
#include <LedLayer.h>
#include <PCRenderer.h>

#define NUM_LEDS 60

int main() {
    LedLayer::PCRenderer renderer(NUM_LEDS);
    LedLayer::LinearLayout layout(NUM_LEDS);
    LedLayer::Display<1> display(renderer, layout);

    float sensorValue = 0.5f;

    LedLayer::LayerConfig gaugeLayer;
    gaugeLayer.source = &sensorValue;
    gaugeLayer.mode = LedLayer::ModeType::COLOR_VALUE_GRADIENT;
    gaugeLayer.gradient.from = {0, 255, 0};
    gaugeLayer.gradient.to = {255, 0, 0};
    display.addLayer(gaugeLayer);

    display.begin();

    for (int i = 0; i < 100; ++i) {
        sensorValue = (sin(i / 10.0f) + 1.0f) / 2.0f;
        display.tick(i * 10);
        const auto& leds = renderer.getLeds();
        std::cout << "Tick " << i << ": ";
        for (int j = 0; j < 5; ++j) {
            std::cout << "(" << int(leds[j].r) << "," << int(leds[j].g) << "," << int(leds[j].b) << ") ";
        }
        std::cout << std::endl;
    }

    return 0;
}
