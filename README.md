# Pulse

**Pulse is a real-time audio visualizer for Windows, built with C++ and JUCE.**

It analyzes system audio and turns it into a live visual representation with a frequency spectrum, stereo goniometer, and level metering.

The project is inspired by the visual style and experience of modern audio monitoring tools, with a focus on a clean, responsive interface.

> **Status:** Active development

---

## ✨ Features

### Real-time Spectrum Analyzer

* Live frequency spectrum visualization
* FFT-based audio analysis
* Logarithmic frequency distribution
* Fast live spectrum response
* Slower reference trace for visual persistence
* Frequency scale from low to high frequencies

### 🎚️ Level Metering

* Real-time audio level monitoring
* Dynamic response to incoming audio
* Designed for continuous audio visualization

### 🎛️ Stereo Goniometer

* Real-time stereo field visualization
* Mid/Side-based stereo representation
* Visual response to left/right stereo movement
* Circular stereo field display

### 🎨 Interface

* Dark audio-monitoring style interface
* Custom Pulse visual theme
* Dedicated visualization components
* Designed around real-time visual feedback

---

## 🖥️ Platform

Currently developed and tested for:

* **Windows**

The project uses JUCE and CMake, with Windows audio support provided through the JUCE audio-device layer.

---

## 🛠️ Technology

Pulse is built using:

* **C++17**
* **JUCE 9.0.1**
* **CMake**
* **Visual Studio / MSVC**

JUCE is fetched automatically by CMake, so it does not need to be manually copied into this repository.

---

# 🚀 Installation

## Option 1 — Download a Release

The easiest way to use Pulse is to download the latest Windows release from the **Releases** section of this repository.

1. Open the latest release.
2. Download the Windows `.zip`.
3. Extract it.
4. Run `Pulse.exe`.

> Releases will be added as stable builds become available.

---

# 🔨 Build From Source

## Requirements

Before building Pulse, install:

* Git
* CMake 3.22 or newer
* Visual Studio with C++ desktop development tools
* A working Windows audio device

---

## Clone the repository

```powershell
git clone https://github.com/Jay19050/Pulse.git
cd Pulse
```

## Configure the project

```powershell
cmake -S . -B build -G "Visual Studio 18 2026"
```

## Build Pulse

```powershell
cmake --build build --config Release
```

## Run Pulse

```powershell
.\build\Pulse_artefacts\Release\Pulse.exe
```

### One-command build

After cloning, the complete build process can be performed with:

```powershell
git clone https://github.com/Jay19050/Pulse.git; cd Pulse; cmake -S . -B build -G "Visual Studio 18 2026"; cmake --build build --config Release
```

The generated executable will be located at:

```text
build\Pulse_artefacts\Release\Pulse.exe
```

---

# 📁 Project Structure

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
│   ├── VisualizerComponent.cpp
│   └── VisualizerComponent.h
│
├── CMakeLists.txt
├── .gitignore
├── README.md
└── LICENSE
```

### Main components

| Component             | Purpose                           |
| --------------------- | --------------------------------- |
| `AudioEngine`         | Audio device and audio processing |
| `SpectrumAnalyzer`    | FFT and frequency analysis        |
| `VisualizerComponent` | Spectrum visualization            |
| `GoniometerComponent` | Stereo field visualization        |
| `LevelMeterComponent` | Audio level visualization         |
| `PulseTheme`          | Application visual styling        |
| `MainComponent`       | Main application interface        |

---

# 🎯 Development Status

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
* [x] Custom dark UI

### In development

* [ ] Further spectrum response tuning
* [ ] Lower-latency visualization
* [ ] Improved stereo visualization behavior
* [ ] Improved audio-device compatibility
* [ ] UI refinement
* [ ] Packaged Windows releases
* [ ] Installation/distribution workflow

---

# 🗺️ Roadmap

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

# 🧑‍💻 Development

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

# 🤝 Contributing

Pulse is currently primarily developed as an independent project.

Issues, suggestions, and improvements are welcome.

If you want to contribute, please open an issue first for larger changes so the direction can be discussed before implementation.

## 🙏 Inspiration & Credits

Pulse is inspired by the excellent **[fxsound-mac](https://github.com/okku007/fxsound-mac)** project by **[@okku007](https://github.com/okku007)**.

`fxsound-mac` is a macOS-focused audio visualizer that served as the primary inspiration for the concept and visual direction of Pulse.

Pulse is an independent **Windows implementation inspired by that project**, built from the ground up using C++ and JUCE with the goal of bringing a similar audio-visualization experience to Windows.

### Original Project

* **fxsound-mac:** https://github.com/okku007/fxsound-mac
* **Author:** [@okku007](https://github.com/okku007)

All credit for the original inspiration and concept goes to the original project and its author.

===

## Author

**Jay19050**

GitHub: https://github.com/Jay19050

---

<p align="center">
  Built with C++ and JUCE.
</p>
