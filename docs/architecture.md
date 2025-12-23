# LedLayer Architecture Overview

This document summarizes the architecture for the LedLayer Arduino library. It captures the layer → mode → track design, layout handling, and conflict resolution strategy.

## Purpose and Scope
LedLayer lets a single addressable LED strip or ring display multiple pieces of information simultaneously. The framework sits atop FastLED and focuses on composable visual encodings rather than device I/O. New shapes (spirals, segmented rings, matrices) can be added by defining additional layouts.

## High-Level Pipeline
`[Variables & Sensors] → [Layers] → [Modes] → [Tracks (FrameModel)] → [Layout-Aware Renderer] → LEDs → FastLED.show()`

- **Variables/Sensors:** Raw inputs (float, int, bool, enum, time) coming from sensors or software.
- **Layers:** Interpret raw inputs with mapping, scaling, thresholds, and optional filters; each layer chooses one mode.
- **Modes:** Visual encodings that write to a specific track (color, brightness, mask, motion, overlay).
- **Tracks:** Per-frame storage for each channel; track rules define whether inputs combine or remain exclusive.
- **Layout-Aware Renderer:** Maps tracks into pixels for the chosen layout (strip or ring), handling clamping or wrapping.
- **Notifications:** Temporary overrides (flash, pulse, chase) that supersede normal rendering and then revert.

## Layouts
- **LinearLayout:** 1-D strip of `n` LEDs; normalized positions map linearly with clamping at ends.
- **RingLayout:** Circular ring of `n` LEDs with optional offset and direction; normalized positions wrap for arcs and clock hands.
- **Extensibility:** Additional shapes can subclass `Layout` and override coordinate mapping.

## Layers (Information Definitions)
Each layer encapsulates:
1. **Source:** Pointer or callback to read the current raw value.
2. **Mapping:** Conversion from raw input to standardized form (continuous mapping with clamp/wrap and curves; discrete thresholds/bins for enums).
3. **Filters (optional):** EMA smoothing, hysteresis for discrete thresholds, debounce for booleans.
4. **Mode:** Exactly one visual encoding (see Modes section).
5. **Mode parameters:** Palette choices, start positions, bar direction, etc.
6. **Priority:** For resolving conflicts on exclusive tracks.

## Tracks (Output Channels)
Tracks represent independent dimensions of the output. Modes write to one track. Default tracks include:

- **COLOR (exclusive):** Base hue/palette.
- **BRIGHTNESS (combinable):** Global intensity and dimming/limiting.
- **MASK (exclusive):** Which LEDs are lit and where (bar/arc length, density, tick marks).
- **MOTION (exclusive):** Animation patterns and speed.
- **OVERLAY (combinable):** Markers and clock hands drawn atop other tracks.

Combination rules:
- Exclusive tracks: Highest-priority layer wins.
- Combinable tracks: BRIGHTNESS multiplies or limits output; OVERLAY accumulates markers (blending by max/brightest).

## Modes (Visual Encodings)
Modes describe how a standardized layer value is encoded visually. They map to one track.

### Color Modes (COLOR track, exclusive)
- State → Palette (discrete states to colors)
- Binary → On/Off Color
- Value → Gradient
- Value → Hue Wheel
- Category → Theme Palette

### Brightness Modes (BRIGHTNESS track, combinable)
- Value → Global Brightness (0..1 scalar)
- Binary → Dim/Boost
- Value → Gamma/Curve Brightness
- Limiter/Night Mode (min cap via ambient sensor)

### Mask Modes (MASK track, usually exclusive)
- Value → Fill (bar/arc)
- Value → Center Fill (symmetric expansion)
- Value → Window Position (sliding segment)
- Discrete → Tick Count
- Binary → Segment Enable
- Value → Density (distributed lit pixels)

### Motion Modes (MOTION track, exclusive pattern + optional speed)
- Solid (no motion)
- Pulse/Breath
- Blink
- Chase/Comet
- Scanner (back-and-forth)
- Twinkle/Sparkle
- Value → Motion Speed (paired with selected pattern)

### Overlay Modes (OVERLAY track, combinable)
- Single Pixel Marker
- Thick Marker
- Threshold Markers (min/max/target)
- Clock Hands (seconds/minute/hour)
- Cardinal Ticks / Quadrant Markers

## Conflict Resolution
- Conflicts on exclusive tracks are resolved at runtime. The layer with the highest priority that is processed in the `tick()` method will be displayed.
- Combinable tracks merge according to their rules (e.g., brightness multiplication, overlay blending).

## Renderer & Layout Integration
- Normalized positions (`0..1`) are used throughout to stay layout-agnostic.
- Per frame, the renderer: (1) chooses base color; (2) applies motion; (3) applies mask to decide lit LEDs; (4) scales brightness; (5) draws overlays last.
- Linear strips map fills to bars and markers to indices; rings map fills to arcs and markers/ticks around the circle.

## Notifications
- Managed separately as temporary overrides with type (flash, pulse, chase), mode (override or overlay), color, duration, and priority.
- Notifications queue by priority and revert to the baseline composition after completion.
