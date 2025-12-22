# LedLayer: Layered LED Display Framework (Scaffold)

LedLayer is a planned Arduino library for composing multiple pieces of information on a single addressable LED strip or ring. This repository currently provides scaffolding only—no functional library code—so contributors can align on the architecture, file layout, and contribution workflow before implementation begins.

## Vision
- **Layer → Mode → Track pipeline:** Inputs (sensors/variables) feed layers, layers choose modes, modes write to dedicated tracks (color, brightness, mask, motion, overlay), and a layout-aware renderer maps tracks to LEDs.
- **Layout abstraction:** Linear strips and circular rings are first-class targets; additional shapes can be added by implementing new layouts.
- **Conflict-aware composition:** Exclusive tracks (e.g., color, mask, motion pattern) resolve via priority, while combinable tracks (brightness, overlays) merge contributions.
- **Notifications:** Temporary overrides (flash, pulse, chase) can supersede the normal composition without altering layer state.

For full conceptual details, see [docs/architecture.md](docs/architecture.md).

## Repository Structure
- `docs/` — Architecture notes, glossary, and contributor guidance.
- `src/` — Placeholder for the future Arduino library sources (intentionally empty).
- `examples/` — Placeholder for example sketches (intentionally empty).
- `library.properties` — Arduino Library Manager metadata stub.
- `.gitignore` — Common ignores for Arduino/C++ builds and local tooling.

## Getting Started
1. **Clone and inspect:** Review the architecture notes in `docs/` to understand the planned pipeline.
2. **Propose changes:** Open issues or PRs to discuss API design, naming, or additional modes/layouts before implementing code.
3. **Implement carefully:** Once coding begins, adhere to the documented tracks and conflict-resolution rules, and keep layouts/modes decoupled from sensor acquisition.

## Status
This is an initial scaffolding commit. Implementation work has not started; any `.cpp`/`.h` additions should come with clear alignment to the architecture documents and tests or simulators where applicable.
