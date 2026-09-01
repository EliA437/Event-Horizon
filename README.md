# Event-Horizon

This project is a production level wavetable synthesizer plugin engineered specifically for electronic dance music producers and sound designers. Built to streamline advanced harmonic workflows in digital audio workstations like FL Studio, the synthesizer bridges the gap between complex sound generation and intuitive chord arrangement. At its core, the plugin features a high performance digital signal processing engine written in modern C++20, ensuring ultra low latency audio generation and stable multithreaded performance even during demanding live stage productions.

The defining feature of this synthesizer is its deeply integrated harmonic database. Powered by a statically linked SQLite engine, the plugin houses a massive local library of popular chords, complex voicings, and progression structures. Users can instantly access, query, and trigger these chords without ever leaving the synthesizer interface. Building upon this database, the plugin features an algorithmic chord generator capable of creating novel harmonic combinations and variations on the fly, allowing producers to map entire complex progressions to single MIDI note inputs on the piano roll.

To achieve a highly responsive and visually rich user experience, the graphical interface is built entirely in C++ using VSTGUI, directly mirroring the architectural choices behind industry standard synthesizers. This allows for deep low level optimization of the rendering pipeline, enabling fluid sixty frames per second waveform visualizations and real time parameter feedback without the overhead of web wrappers. The entire codebase is managed via CMake, heavily optimized for local native compilation on Apple Silicon machines like the Mac Mini, and structured to deliver a truly professional grade audio tool.

## Building

### Prerequisites

- macOS on Apple Silicon
- Xcode Command Line Tools (`xcode-select --install`)
- [Homebrew](https://brew.sh), then:

```bash
brew install cmake ninja
```

### Clone with submodules

The Steinberg VST3 SDK (including VSTGUI) lives at `external/vst3sdk` and must be initialized recursively:

```bash
git clone --recurse-submodules <repo-url>
# or, if you already cloned:
git submodule update --init --recursive
```

### Configure and build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The build produces `build/VST3/Debug/EventHorizon.vst3` (Release builds land under `build/VST3/Release/`) and, via `SMTG_CREATE_PLUGIN_LINK`, a symlink in `~/Library/Audio/Plug-Ins/VST3/`. Rescan plugins in your DAW (e.g. FL Studio) to load **EventHorizon** as an instrument.

> **Note:** A full Xcode.app is preferred. If only Command Line Tools are installed, CMake seeds `XCODE_VERSION` automatically so the VST3 SDK can configure.
