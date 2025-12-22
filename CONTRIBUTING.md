# Contributing to LedLayer

Thanks for your interest in building the LedLayer library. This repository is currently a scaffold without implementation code. Please align on the architecture before adding functionality.

## Ground Rules
- **Discuss design first:** Open an issue or draft PR describing proposed APIs, mode behavior, or layout extensions before writing code.
- **Keep layers semantic:** Layers represent information sources; modes define encodings; tracks define how outputs combine.
- **Separation of concerns:** Rendering/layout code should stay independent of sensor I/O and device setup.
- **Arduino-friendly:** Favor small footprints and compile-time limits suitable for AVR-class boards.

## Workflow
1. Fork the repository and create a feature branch.
2. Add or update documents/tests alongside code changes.
3. Keep commits focused and descriptive; include rationale when deviating from the documented architecture.
4. Submit a PR referencing any related issues and describing track/mode impacts.

## File/Directory Conventions
- `src/` will hold library headers and sources once implementation begins.
- `examples/` will contain Arduino sketches demonstrating common compositions and layouts.
- `docs/` contains design references; update these when introducing new tracks, modes, or layouts.

## Reporting Issues
Please include:
- Hardware target (e.g., AVR, SAMD, ESP32) and LED type.
- Reproduction steps or configuration snippets.
- Expected vs. actual behavior.

## Code of Conduct
Please be respectful and constructive in discussions and reviews. Decisions should prioritize clarity, safety, and maintainability for hobbyists and embedded developers alike.
