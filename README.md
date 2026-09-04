# Pulse

**Pulse is a real-time audio visualizer for Windows, built with C++ and JUCE.**

It analyzes incoming audio and turns it into a live visual representation with a frequency spectrum, stereo goniometer, and level metering.

The project focuses on creating a clean, responsive audio-visualization experience for Windows.

> **Status:** Active development

---

## ✨ Features

### 🎵 Real-time Spectrum Analyzer

- Live frequency spectrum visualization
- FFT-based audio analysis
- Logarithmic frequency distribution
- Fast live spectrum response
- Slower reference trace for visual persistence
- Frequency scale covering the audible spectrum

### 🎚️ Level Metering

- Real-time audio level monitoring
- Dynamic response to incoming audio
- Designed for continuous audio visualization

### 🎛️ Stereo Goniometer

- Real-time stereo-field visualization
- Mid/Side-based stereo representation
- Visual response to left/right stereo movement
- Circular stereo-field display

### 🎨 Interface

- Dark audio-monitoring style interface
- Custom Pulse visual theme
- Dedicated visualization components
- Designed around real-time visual feedback

---

## 🙏 Inspiration & Credits

Pulse is inspired by the excellent **[fxsound-mac](https://github.com/okku007/fxsound-mac)** project by **[@okku007](https://github.com/okku007)**.

`fxsound-mac` is a macOS-focused audio visualizer that served as the primary inspiration for the concept and visual direction of Pulse.

Pulse is an independent **Windows implementation inspired by that project**, built from the ground up using C++ and JUCE with the goal of bringing a similar audio-visualization experience to Windows.

### Original Project

- **fxsound-mac:** https://github.com/okku007/fxsound-mac
- **Author:** [@okku007](https://github.com/okku007)

All credit for the original inspiration and concept goes to the original project and its author.

Pulse is **not affiliated with or endorsed by** the original `fxsound-mac` project or its author.

---

## 🖥️ Platform

Currently developed for:

- **Windows**

The project uses JUCE for its application and audio functionality.

---

## 🛠️ Technology

Pulse is built using:

- **C++17**
- **JUCE 9.0.1**
- **CMake**
- **Visual Studio / MSVC**

JUCE is fetched automatically by CMake, so it does not need to be manually copied into this repository.

---

# 🚀 Installation

## Option 1 — Download a Release

The easiest way to use Pulse is to download the latest Windows release from the **Releases** section of this repository.

1. Open the latest release.
2. Download the Windows `.zip`.
3. Extract the archive.
4. Run `Pulse.exe`.

> Pre-built releases will be provided as stable versions become available.

---

# 🔨 Build From Source

## Requirements

Before building Pulse, install:

- Git
- CMake 3.22 or newer
- Visual Studio with the C++ Desktop Development workload
- A working Windows audio device

---

## Clone the Repository

```powershell
git clone https://github.com/Jay19050/Pulse.git
cd Pulse
