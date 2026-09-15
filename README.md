# Pulse

**Pulse is a real-time audio visualizer for Windows, built with C++ and JUCE.**

It analyzes system audio and turns it into a live visual representation with a frequency spectrum, stereo goniometer, and level metering.

The project is inspired by the visual style and experience of modern audio monitoring tools, with a focus on a clean, responsive interface.

> **Status:** Active development

---

##  Features

### Real-time Spectrum Analyzer

* Live frequency spectrum visualization
* FFT-based audio analysis
* Logarithmic frequency distribution
* Fast live spectrum response
* Slower reference trace for visual persistence
* Frequency scale from low to high frequencies

###  Multiple Visualizer Modes

* Six switchable real-time visualizer modes, all driven by the same shared audio-analysis pipeline (no per-mode FFT):
  * **Spectrum** — the original LIVE / AVG / PEAK HOLD view, with a collapsible legend, hover readout, and per-trace visibility toggles
  * **Filled Spectrum** — a clean, single-trace filled version of the spectrum
  * **Waveform** — a real-time scrolling oscilloscope of the raw audio signal
  * **Circular** — a 360° radial spectrum with a peak-hold ring
  * **Mirror** — a symmetrical bar spectrum mirrored above/below a centerline
  * **Particles** — a layered, flowing audio-reactive particle field: background dust, flowing ribbon streams, a level-driven central energy vortex, and high-frequency sparks — all deterministic functions of the real spectrum, not a random particle simulation
* Compact mode-selector bar with icons — switch modes live, no restart required
* Every mode goes idle/flat automatically when there's no audio — nothing is faked or randomly animated

###  Output Device Selector

* Lists available Windows render (output) devices via WASAPI enumeration
* Switch which device Pulse listens to while running — no restart required
* Refresh the device list on demand
* Falls back to the system default if the selected device disconnects

###  True Fullscreen

* **F11** toggles genuine borderless fullscreen — no title bar, no window frame, no taskbar
* **Esc** exits fullscreen
* Restores the previous window size/position (and maximized state) on exit

###  Visualizer Appearance Settings

* Sensitivity, Smoothing, Brightness, Peak Intensity, and Background controls, applied live across all six modes
* Custom-styled sliders matching Pulse's own visual language
* Visual-only — never touches captured audio, system volume, or Windows audio settings

###  Level Metering

* Real-time audio level monitoring
* Dynamic response to incoming audio
* Designed for continuous audio visualization

###  Stereo Goniometer

* Real-time stereo field visualization
* Mid/Side-based stereo representation
* Visual response to left/right stereo movement
* Circular stereo field display

###  Interface

* Dark, premium, custom Pulse visual theme — header, mode selector bar, and settings drawer all match
* Header with logo, LIVE/IDLE audio-activity indicator, and current device readout
* Slide-out settings drawer (device selector, fullscreen, visualizer appearance)
* Designed around real-time visual feedback

---

##  Platform

Currently developed and tested for:

* **Windows**

The project uses JUCE and CMake, with Windows audio support provided through the JUCE audio-device layer.

---

##  Technology

Pulse is built using:

* **C++17**
* **JUCE 9.0.1**
* **CMake**
* **Visual Studio / MSVC**

JUCE is fetched automatically by CMake, so it does not need to be manually copied into this repository.

---

#  Installation

## Option 1 — Download a Release

The easiest way to use Pulse is to download the latest Windows release from the **Releases** section of this repository.

1. Open the latest release.
2. Download the Windows `.zip`.
3. Extract it.
4. Run `Pulse.exe`.

> Releases will be added as stable builds become available.

---

#  Build From Source

## Requirements

Before building Pulse, install:

* Git
* CMake 3.22 or newer
* Visual Studio with C++ desktop development tools
* A working Windows audio device

---


#  Project Structure

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
├── README.md
└── LICENSE
```

### Main components

| Component               | Purpose                                           |
| ------------------------ | -------------------------------------------------- |
| `AudioEngine`            | WASAPI loopback capture, output-device enumeration/selection |
| `SpectrumAnalyzer`       | FFT and frequency analysis (shared by every mode)  |
| `VisualizerMode`         | Common interface implemented by every visualizer mode |
| `VisualizerSettings`     | Shared visual-only appearance parameters (sensitivity, smoothing, brightness, peak intensity, background) |
| `VisualizerModeManager`  | Owns all modes, handles switching                  |
| `VisualizerComponent`    | Spectrum mode (LIVE / AVG / PEAK HOLD + legend)    |
| `ModeFilledSpectrum`     | Filled Spectrum mode                               |
| `ModeWaveform`           | Waveform (oscilloscope) mode                       |
| `ModeCircular`           | Circular spectrum mode                             |
| `ModeMirror`             | Mirror spectrum mode                               |
| `ModeParticles`          | Layered flowing particle-field mode                |
| `WaveformBuffer`         | Rolling raw-sample buffer feeding Waveform mode     |
| `GoniometerComponent`    | Stereo field visualization                         |
| `LevelMeterComponent`    | Audio level visualization                          |
| `HeaderBar`              | Logo, LIVE/IDLE indicator, device readout, settings button |
| `ModeSelectorBar`        | Bottom mode-selector bar                           |
| `SettingsPanel`          | Slide-out drawer: device selector, fullscreen, appearance sliders |
| `DeviceSelectorControl`  | Custom output-device dropdown                      |
| `AppearanceSlider`       | Custom-styled slider used in the settings drawer   |
| `IconButton`             | Shared gear/close icon button                      |
| `PulseTheme`             | Application visual styling                         |
| `MainComponent`          | Main application interface                         |

---

#  Development Status

Pulse is currently under active development.

### Working

* [x] CMake project
* [x] JUCE integration
* [x] Windows application
* [x] Real-time audio input
* [x] FFT spectrum analysis
* [x] Spectrum visualization
* [x] Stereo goniometer
* [x] Level metering
* [x] Custom dark UI (header, mode selector bar, settings drawer)
* [x] Multiple visualizer modes (Spectrum, Filled Spectrum, Waveform, Circular, Mirror, Particles)
* [x] Live mode switching (no restart required)
* [x] Output device selection, switching, and disconnect fallback
* [x] True borderless fullscreen (F11 / Esc)
* [x] Visualizer appearance settings (Sensitivity, Smoothing, Brightness, Peak Intensity, Background)
* [x] Layered, flowing Particles mode

### In development

* [ ] Further spectrum response tuning
* [ ] Lower-latency visualization
* [ ] Improved stereo visualization behavior
* [ ] Improved audio-device compatibility
* [ ] Performance settings (FPS limit, render quality)
* [ ] Preference persistence across launches
* [ ] Packaged Windows releases
* [ ] Installation/distribution workflow

---

#  Roadmap

The long-term goal is to turn Pulse into a polished desktop audio visualization and monitoring application.

Planned areas include:

* More responsive visual animations
* Improved spectrum rendering
* More accurate stereo-field visualization
* Better audio-device handling
* Configurable visualization settings
* Additional audio-analysis tools
* Improved Windows distribution
* Installer support
* Performance optimization

---

#  Development

Clone the repository:

```powershell
git clone https://github.com/Jay19050/Pulse.git
cd Pulse
```

Configure:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026"
```

Build:

```powershell
cmake --build build --config Release
```

After making changes:

```powershell
cmake --build build --config Release
```

---

#  Contributing

Pulse is currently primarily developed as an independent project.

Issues, suggestions, and improvements are welcome.

If you want to contribute, please open an issue first for larger changes so the direction can be discussed before implementation.

##  Inspiration & Credits

Pulse is inspired by the excellent **[fxsound-mac](https://github.com/okku007/fxsound-mac)** project by **[@okku007](https://github.com/okku007)**.

`fxsound-mac` is a macOS-focused audio visualizer that served as the primary inspiration for the concept and visual direction of Pulse.

The first version of Pulse — the native Windows/WASAPI port with the Spectrum visualizer, stereo goniometer, and level metering — was **[Jay](https://github.com/Jay19050)'s own build**, in C++ and JUCE, inspired by `fxsound-mac`'s concept and visual direction.

**Multiple visualizer modes** (Filled Spectrum, Waveform, Circular, Mirror, and Particles), live mode switching, and the shared `VisualizerMode` architecture behind them were then added on top of that original Spectrum build, reusing the same audio pipeline without modifying it.

### Original Project

* **fxsound-mac:** https://github.com/okku007/fxsound-mac
* **Author:** [@okku007](https://github.com/okku007)

All credit for the original inspiration and concept goes to the original project and its author.

===

## Author

**Jay19050**

GitHub: https://github.com/Jay19050

Built the original Windows/JUCE Spectrum visualizer, and added the multiple visualizer modes, output device selection, true fullscreen, and the visualizer appearance settings system.

---

<p align="center">
  Built with C++ and JUCE.
</p>
