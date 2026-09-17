# Pulse

**Pulse is a real-time audio visualizer for Windows, built with C++ and JUCE.**

Pulse analyzes system audio and turns it into a live visual representation with a frequency spectrum, multiple visualization modes, stereo goniometer, and level metering.

> **Status:** v0.1.0 — Windows Release

---

## Features

### Real-time Spectrum Analyzer

- Live frequency spectrum visualization
- FFT-based audio analysis
- Logarithmic frequency distribution
- Fast live spectrum response
- Slower reference trace for visual persistence
- Frequency scale from low to high frequencies
- Continuous graph-style visualization rather than a traditional equalizer

### Multiple Visualizer Modes

Six switchable real-time visualizer modes, all driven by the same shared audio-analysis pipeline:

- **Spectrum** — LIVE / AVG / PEAK HOLD spectrum view with legend, hover readout, and per-trace visibility controls
- **Filled Spectrum** — filled spectrum visualization
- **Waveform** — real-time scrolling oscilloscope of the audio signal
- **Circular** — 360° radial spectrum with peak-hold ring
- **Mirror** — symmetrical spectrum mirrored around a centerline
- **Particles** — layered audio-reactive particle field with flowing streams, central energy vortex, background particles, and high-frequency sparks

Additional mode features:

- Compact mode-selector bar
- Switch modes live without restarting Pulse
- Shared audio analysis between all modes
- Every mode becomes idle/flat when there is no audio
- No fake or random audio-independent visualization

### Output Device Selector

- Lists available Windows render/output devices
- Uses WASAPI for system-audio loopback
- Switches the device Pulse listens to while running
- Refreshes the device list
- Handles disconnected devices and falls back to the system default

### True Fullscreen

- **F11** toggles genuine borderless fullscreen
- Removes the title bar, window frame, and taskbar from the fullscreen view
- **Esc** exits fullscreen
- Restores the previous window size and position

### Visualizer Appearance Settings

- Sensitivity
- Smoothing
- Brightness
- Peak Intensity
- Background Intensity

All appearance controls are applied live across the visualizer modes.

These controls are visual-only and do **not** modify:

- System volume
- Captured audio
- Audio routing
- Equalization
- Audio effects
- DSP processing

### Level Metering

- Real-time audio level monitoring
- Left/right audio monitoring
- Dynamic response to incoming audio
- Designed for continuous audio visualization

### Stereo Goniometer

- Real-time stereo field visualization
- Mid/Side-based stereo representation
- Response to left/right stereo movement
- Circular stereo field display

### Interface

- Dark, custom Pulse visual theme
- Header with Pulse branding
- LIVE / IDLE audio activity indicator
- Current output device readout
- Slide-out settings drawer
- Output device selection
- Fullscreen control
- Visualizer appearance controls
- Compact bottom mode-selector bar

---

# How Pulse Works

Pulse listens to audio already being played through a Windows output device using WASAPI loopback capture.

```text
System Audio
     │
     ▼
Windows Render Device
     │
     ▼
WASAPI Loopback
     │
     ▼
Audio Analysis
     │
     ├── FFT Spectrum
     ├── Waveform
     ├── Level Metering
     └── Stereo Analysis
     │
     ▼
Visualizer Modes
```

Pulse is a **visualizer only**. It does not replace the Windows audio device, modify system volume, or process the audio signal for playback.

---

# Platform

Currently developed and tested for:

- **Windows 10 / Windows 11**
- **x64**

Pulse uses JUCE's audio-device layer for Windows audio capture and device handling.

---

# Technology

Pulse is built using:

- **C++17**
- **JUCE 9.0.1**
- **CMake**
- **Visual Studio / MSVC**
- **WASAPI**

JUCE is fetched automatically by CMake, so it does not need to be manually copied into this repository.

---

# Installation

## Option 1 — Download the Windows Installer

The easiest way to install Pulse is through the Windows installer available in the **Releases** section of this repository.

1. Open the latest Pulse release.
2. Download `PulseSetup.exe`.
3. Run the installer.
4. Follow the installation instructions.
5. Launch Pulse from the Start Menu or desktop shortcut.

The installer handles the application installation and creates the required shortcuts.

## Option 2 — Build From Source

If you want to compile Pulse yourself, follow the build instructions below.

---

# Build From Source

## Requirements

Before building Pulse, install:

- Git
- CMake 3.22 or newer
- Visual Studio with Desktop development with C++ installed
- A working Windows audio output device

## Clone the Repository

```powershell
git clone https://github.com/Jay19050/Pulse.git
cd Pulse
```

## Configure

```powershell
cmake -S . -B build -G "Visual Studio 18 2026"
```

## Build

```powershell
cmake --build build --config Release
```

The resulting executable will be located in:

```text
build/Pulse_artefacts/Release/Pulse.exe
```

## Rebuild After Changes

```powershell
cmake --build build --config Release
```

---

# Project Structure

```text
Pulse/
│
├── Source/
│   ├── AudioEngine.cpp
│   ├── AudioEngine.h
│   ├── GoniometerComponent.cpp
│   ├── GoniometerComponent.h
│   ├── LevelMeterComponent.cpp
│   ├── LevelMeterComponent.h
│   ├── Main.cpp
│   ├── MainComponent.cpp
│   ├── MainComponent.h
│   ├── PulseTheme.h
│   ├── Pulse.ico
│   ├── Pulse.png
│   ├── Pulse.rc
│   ├── SpectrumAnalyzer.cpp
│   ├── SpectrumAnalyzer.h
│   ├── VisualizerMode.h
│   ├── VisualizerSettings.h
│   ├── VisualizerComponent.cpp
│   ├── VisualizerComponent.h
│   ├── VisualizerModeManager.cpp
│   ├── VisualizerModeManager.h
│   ├── CurveUtils.h
│   ├── WaveformBuffer.cpp
│   ├── WaveformBuffer.h
│   ├── ModeFilledSpectrum.cpp
│   ├── ModeFilledSpectrum.h
│   ├── ModeWaveform.cpp
│   ├── ModeWaveform.h
│   ├── ModeCircular.cpp
│   ├── ModeCircular.h
│   ├── ModeMirror.cpp
│   ├── ModeMirror.h
│   ├── ModeParticles.cpp
│   ├── ModeParticles.h
│   ├── HeaderBar.cpp
│   ├── HeaderBar.h
│   ├── ModeSelectorBar.cpp
│   ├── ModeSelectorBar.h
│   ├── SettingsPanel.cpp
│   ├── SettingsPanel.h
│   ├── DeviceSelectorControl.cpp
│   ├── DeviceSelectorControl.h
│   ├── AppearanceSlider.cpp
│   ├── AppearanceSlider.h
│   ├── IconButton.cpp
│   └── IconButton.h
│
├── CMakeLists.txt
├── .gitignore
├── Pulse.iss
├── README.md
└── LICENSE
```

### Main Components

| Component | Purpose |
|---|---|
| `AudioEngine` | WASAPI loopback capture and output-device handling |
| `SpectrumAnalyzer` | FFT and frequency analysis shared by the visualizer |
| `VisualizerMode` | Common interface for visualizer modes |
| `VisualizerSettings` | Shared visual appearance parameters |
| `VisualizerModeManager` | Owns and switches visualizer modes |
| `VisualizerComponent` | Main Spectrum mode |
| `ModeFilledSpectrum` | Filled Spectrum mode |
| `ModeWaveform` | Waveform/oscilloscope mode |
| `ModeCircular` | Circular spectrum mode |
| `ModeMirror` | Mirror spectrum mode |
| `ModeParticles` | Audio-reactive particle-field mode |
| `WaveformBuffer` | Rolling audio sample buffer |
| `GoniometerComponent` | Stereo field visualization |
| `LevelMeterComponent` | Audio level visualization |
| `HeaderBar` | Branding, audio status, device information, settings |
| `ModeSelectorBar` | Visualizer mode selection |
| `SettingsPanel` | Device, fullscreen, and appearance controls |
| `DeviceSelectorControl` | Output-device selector |
| `AppearanceSlider` | Custom appearance slider |
| `IconButton` | Shared interface icon button |
| `PulseTheme` | Application visual styling |
| `MainComponent` | Main application interface |

---

# Development Status

## Working

- [x] CMake project
- [x] JUCE integration
- [x] Windows application
- [x] WASAPI system-audio loopback
- [x] Real-time audio analysis
- [x] FFT spectrum analysis
- [x] Spectrum visualization
- [x] Stereo goniometer
- [x] Left/right level metering
- [x] Custom dark UI
- [x] Multiple visualizer modes
- [x] Live mode switching
- [x] Output device selection
- [x] Device switching without restart
- [x] Device disconnect fallback
- [x] True borderless fullscreen
- [x] F11 / Esc fullscreen controls
- [x] Visualizer appearance settings
- [x] Layered Particles mode
- [x] Windows application icon
- [x] Windows installer
- [x] Release packaging

## In Development

- [ ] Further spectrum response tuning
- [ ] Lower-latency visualization
- [ ] Improved stereo visualization behavior
- [ ] Improved audio-device compatibility
- [ ] Performance settings
- [ ] Preference persistence across launches
- [ ] Additional visualization improvements

---

# Roadmap

The long-term goal is to continue developing Pulse into a polished lightweight desktop audio visualization and monitoring application.

Planned areas include:

- More responsive visual animations
- Further spectrum rendering improvements
- More accurate stereo-field visualization
- Improved audio-device handling
- Additional visualization modes
- Configurable performance settings
- Preference persistence
- Additional audio-analysis tools
- Further Windows distribution improvements

---

# Contributing

Pulse is currently primarily developed as an independent project.

Issues, suggestions, and improvements are welcome.

If you want to contribute, please open an issue first for larger changes so the direction can be discussed before implementation.

---

# Inspiration & Credits

Pulse is inspired by the excellent **[fxsound-mac](https://github.com/okku007/fxsound-mac)** project by **[@okku007](https://github.com/okku007)**.

`fxsound-mac` is a macOS-focused audio visualizer that served as an important inspiration for the concept and visual direction of Pulse.

The first version of Pulse — the native Windows/WASAPI implementation with the Spectrum visualizer, stereo goniometer, and level metering — was built independently in C++ and JUCE, inspired by the concept and visual direction of `fxsound-mac`.

Multiple visualizer modes, live mode switching, output-device selection, fullscreen support, and the shared visualizer architecture were subsequently developed on top of the original Spectrum implementation.

### Original Project

- **fxsound-mac:** https://github.com/okku007/fxsound-mac
- **Author:** [@okku007](https://github.com/okku007)

All credit for the original inspiration and concept goes to the original project and its author.

---

# Author

**Jay19050**

GitHub: https://github.com/Jay19050

Built the original Windows/JUCE Spectrum visualizer and developed the multiple visualizer modes, output-device selection, fullscreen functionality, and visualizer appearance settings system.

---

<p align="center">
  Built with C++ and JUCE.
</p>
