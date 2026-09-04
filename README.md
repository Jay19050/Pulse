# Pulse

<p align="center">
  <strong>Real-time audio visualization for Windows</strong>
</p>

<p align="center">
  A lightweight C++ / JUCE audio visualizer featuring a live FFT spectrum and stereo goniometer.
</p>

---

## 🎧 About

**Pulse** is a real-time audio visualization application built for Windows using **C++17** and the **JUCE framework**.

Pulse analyzes incoming system audio and transforms it into live visualizations, including:

- Real-time frequency spectrum
- Fast and slow spectrum traces
- Stereo goniometer
- Audio level monitoring
- Logarithmic frequency distribution

The goal is to create a clean, responsive desktop audio visualization experience for Windows.

---

## ✨ Features

### 📊 Real-Time Spectrum Analyzer

Pulse uses FFT-based analysis to visualize the frequency content of incoming audio.

Features include:

- Real-time FFT processing
- Logarithmic frequency distribution
- 20 Hz to the upper audible frequency range
- Fast live spectrum response
- Slower reference spectrum trace
- Smooth visual decay
- Dynamic frequency response

### 🎛️ Stereo Goniometer

The goniometer provides a real-time representation of the stereo field.

It can be used to visually observe:

- Stereo width
- Left/right balance
- Centered signals
- Stereo movement
- Correlation between channels

### 🔊 Audio Level Detection

Pulse continuously analyzes the incoming audio level and uses it to drive the visual interface.

### 🖥️ Windows Application

Pulse is currently designed specifically for:

**Windows**

The project uses JUCE for audio-device handling and the graphical interface.

---

# 🖼️ Preview

Pulse's interface contains the main spectrum visualization together with the stereo goniometer and audio monitoring area.

> Screenshots and demo media will be added as the project develops.

---

# 🙏 Inspiration & Credits

Pulse is inspired by the excellent **fxsound-mac** project created by **@okku007**.

The original project is a macOS-focused audio visualizer and was the primary inspiration for the concept and visual direction of Pulse.

Pulse is an independent **Windows implementation inspired by that project**, built using C++ and JUCE with the goal of bringing a similar audio-visualization experience to Windows.

### Original Project

**fxsound-mac**  
Author: **@okku007**

Repository:

`okku007/fxsound-mac`

All credit for the original inspiration and concept goes to the original project and its author.

Pulse is **not affiliated with, sponsored by, or endorsed by** the original `fxsound-mac` project or its author.

---

# 🛠️ Technology

Pulse is built using:

- **C++17**
- **JUCE 9.0.1**
- **CMake 3.22+**
- **Visual Studio / MSVC**
- **Windows**

JUCE is downloaded automatically by CMake using `FetchContent`.

You do **not** need to manually download or copy JUCE into the Pulse repository.

---

# 📋 Requirements

Before building Pulse, make sure you have:

### Required

- Windows 10 or newer
- Git
- CMake 3.22 or newer
- Visual Studio 2022 or newer
- Visual Studio C++ Desktop Development workload
- A working Windows audio device

### Visual Studio Workload

During Visual Studio installation, make sure this workload is installed:

**Desktop development with C++**

The installation should include:

- MSVC compiler
- Windows SDK
- CMake tools for Windows

---

# 🚀 Installation

There are currently two ways to use Pulse.

## Option 1 — Download a Release

When a packaged release is available:

1. Open the latest GitHub Release.
2. Download the Windows ZIP file.
3. Extract the ZIP.
4. Run:

```text
Pulse.exe
