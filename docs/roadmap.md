# LedLayer Roadmap (Draft)

This roadmap tracks high-level steps to turn the scaffold into a working library.

## Near Term
- Finalize public API for layers, modes, tracks, and layout selection.
- Define configuration/validation helpers and compile-time limits (e.g., `MAX_LAYERS`).
- Decide default dependency targets (FastLED vs. Adafruit NeoPixel) and abstraction layer.

## Mid Term
- Implement core data structures (Layer, Mode registry, FrameModel, Renderer).
- Add LinearLayout and RingLayout implementations with normalized coordinate mapping.
- Provide initial modes for color, brightness, mask, motion, and overlay tracks.
- Introduce a notification manager that supports override/overlay semantics.
- Ship example sketches demonstrating concurrent indicators (e.g., clock + battery + status).

## Long Term
- Expand layouts (spiral, segmented ring, small matrix) and richer modes.
- Add gamma-correct brightness curves and configurable palettes.
- Provide PC-side simulation/preview tooling for rapid iteration.
- Integrate CI for formatting, linting, and board compilation where feasible.
