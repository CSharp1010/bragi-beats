**Code Review**

---

**Overall Architecture**

The project is a music visualizer application named **Bragi Beats**. It is built using C and [Raylib](https://www.raylib.com/) library for graphics and audio handling.

- **Audio Processing**: Handles FFT computations, test signal generation, and audio data management.
- **Playback Control**: Manages song playback, queue management, and user interaction.
- **UI Rendering**: Manages the user interface, including rendering and input handling.
- **Visualizers**: Contains implementations of audio visualizers.
- **Main Application**: Initializes the application and orchestrates the interaction between modules.

---

**Detailed Module Review**

1. **Audio Processing (`audio_processing.c` and `audio_processing.h`)**

   - **FFT Implementation**: Implements a custom in-place iterative FFT algorithm with precomputed twiddle factors and bit-reversal indices. The code allows switching between different window functions (Hanning and Blackman-Harris) and includes perceptual weighting to enhance the audio analysis.

   - **Test Signal Generation**: Provides functions to generate various test signals such as sine waves, multi-sine waves, chirps, and white noise. This is particularly useful for testing and calibrating the visualizers.

   - **Audio Data Management**: The `AudioData` structure efficiently manages buffers for raw input, windowed input, FFT output, and various spectral analyses.

   - **Performance Considerations**: Precomputing coefficients and indices improves the performance of the FFT operations.

   - **Suggestions**:
     - **Error Handling**: Ensure that all dynamic memory allocations are checked for success to prevent segmentation faults.
     - **Optimization**: Consider utilizing SIMD instructions or parallel processing to further optimize the FFT computation.

2. **Playback Control (`playback.h` and `main.c` implementation)**

   - **Song Queue Management**: Implements a doubly linked list (`SongNode`) to manage the playlist, allowing efficient insertion and removal of songs.

   - **Playback Functions**: Provides functions to play, pause, skip forward, and skip backward through the playlist. The code also handles the automatic transition to the next song when the current one ends.

   - **Media Library Loading**: Recursively loads songs from the `./media` directory, supporting multiple audio formats (`.wav`, `.mp3`) and subdirectories (albums).

   - **Drag-and-Drop Support**: Allows users to add songs to the queue by dragging and dropping files onto the application window.

   - **Suggestions**:
     - **Memory Management**: Ensure that all loaded `Music` streams are properly unloaded to prevent memory leaks.
     - **Format Support**: Expand audio format support to include more file types if needed.

3. **UI Rendering (`ui_renderer.c` and `ui_renderer.h`)**

   - **Layout Management**: Calculates and manages the layout of the UI components dynamically based on the window size.

   - **User Interface Components**: Renders the title bar, playback controls, progress bar, song queue, and visualizer area with a consistent and clean design.

   - **Interactive Elements**: Implements buttons and controls with hover and click effects, enhancing user experience.

   - **Input Handling**: Separates input handling logic for better code organization and readability.

   - **Suggestions**:
     - **Responsive Design**: Ensure that the UI scales well on different screen resolutions.
     - **Accessibility**: Consider adding keyboard navigation and tooltips for better accessibility.

4. **Visualizers (`visualizers.c` and `visualizers.h`)**

   - **Bar Chart Visualizer**: A classic bar chart representation of the audio spectrum, with smooth animations and gradient colors.

   - **Iridescent Visualizer**: A circular visualizer that uses particles and color gradients to create a vibrant and dynamic display.

   - **3D Time Tunnel Visualizer**: A 3D tunnel effect that responds to the audio input. Recent enhancements include smoother walls, lighting and shading, backface culling, and color transitions to improve visual appeal.

   - **Suggestions**:
     - **Performance Optimization**: Use efficient data structures and minimize state changes in rendering loops to improve performance.
     - **Shader Usage**: Implement custom shaders to add advanced visual effects like bloom, glow, or motion blur.

5. **Main Application (`main.c`)**

   - **Initialization**: Sets up the application window, audio device, and initializes the UI and audio processing modules.

   - **Main Loop**: Handles input events, updates playback state, processes audio data through the FFT, and renders the UI each frame.

   - **Cleanup**: Ensures that all resources are properly released when the application exits.

   - **Suggestions**:
     - **Error Handling**: Add checks for potential failures during initialization (e.g., audio device not available).
     - **Modularity**: Consider refactoring some of the main loop logic into separate functions for clarity.

---

**General Observations**

- **Code Quality**: The code is generally well-written with meaningful variable and function names. Comments and documentation are present, which aids in understanding the codebase.

- **Modularity**: Good separation of concerns between modules, making the codebase maintainable and extensible.

- **User Experience**: The application provides a user-friendly interface with intuitive controls and interactive visualizations.

- **Performance**: Real-time audio processing and rendering are handled efficiently, but there is room for optimization, especially in rendering complex visualizations.

---

**Recommendations**

- **Testing**: Implement unit tests for critical components, especially the custom FFT implementation, to ensure correctness.

- **Cross-Platform Support**: Verify that the application runs smoothly on different operating systems and address any platform-specific issues.

- **Documentation**: Enhance inline documentation and consider generating a comprehensive developer guide.

- **Community Engagement**: Encourage contributions by providing clear guidelines and maintaining an open issue tracker.

---

**README.md**

---

# Bragi Beats

**Experience music like never before with Bragi Beats—a captivating audio visualizer that transforms sound into mesmerizing visual art.**

![Bragi Beats Banner](assets/banner.png)

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Screenshots](#screenshots)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
- [Usage](#usage)
- [Customization](#customization)
  - [Visualizers](#visualizers)
  - [Test Signals](#test-signals)
- [Development](#development)
  - [Switching FFT Algorithms](#switching-fft-algorithms)
  - [Performance Measurement](#performance-measurement)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)

---

## Overview

Bragi Beats is a real-time music visualization application built with C and [Raylib](https://www.raylib.com/). It analyzes audio signals using Fast Fourier Transform (FFT) and renders stunning visualizations that react to the nuances of the sound.

Whether you're an audiophile, a developer, or someone who loves visual art, Bragi Beats offers a unique way to experience music.

---

## Features

- **Real-Time Audio Visualization**: Transforms live audio input into dynamic visual displays.

- **Multiple Visualizer Modes**:
  - **Bar Chart**: A classic spectrum analyzer with smooth animations.
  - **Iridescent Visualizer**: A vibrant, circular visualizer with particle effects.
  - **3D Time Tunnel**: An immersive tunnel that pulses and twists to the beat.

- **Custom and Professional FFT Support**: Includes a custom FFT algorithm with the ability to switch to professional libraries like FFTW for comparison.

- **Test Signal Generation**: Generate various test signals (sine wave, multi-sine, chirp, white noise) for analysis and calibration.

- **Intuitive User Interface**:
  - Sleek, modern design inspired by Apple's aesthetic.
  - Responsive playback controls and progress bar.
  - Drag-and-drop support for adding songs.
  - Interactive song queue and visualizer selection.

- **Media Library Integration**: Automatically scans and loads songs from your local media directory.

- **Cross-Platform Compatibility**: Runs on Windows, macOS, and Linux.

---

## Screenshots

*(Add high-quality screenshots showcasing each visualizer and the user interface.)*

- **Bar Chart Visualizer**

  ![Bar Chart Visualizer](assets/bar_chart.png)

- **Iridescent Visualizer**

  ![Iridescent Visualizer](assets/iridescent.png)

- **3D Time Tunnel Visualizer**

  ![3D Time Tunnel Visualizer](assets/time_tunnel.png)

---

## Getting Started

### Prerequisites

- **C Compiler**: GCC or Clang supporting C99 standard or later.

- **Raylib Library**: [Raylib](https://www.raylib.com/) installed on your system.

- **FFTW Library (Optional)**: [FFTW](http://www.fftw.org/) installed if you wish to compare FFT algorithms.

### Installation

1. **Clone the Repository**

   ```bash
   git clone https://github.com/yourusername/BragiBeats.git
   cd BragiBeats
   ```

2. **Install Dependencies**

   - **Raylib Installation**:

     - **macOS** (using Homebrew):

       ```bash
       brew install raylib
       ```

     - **Linux** (Debian/Ubuntu):

       ```bash
       sudo apt-get install libraylib-dev
       ```

     - **Windows**:

       - Download the Windows installer or binaries from the [Raylib website](https://www.raylib.com/).
       - Follow the installation instructions provided.

   - **FFTW Installation (Optional)**:

     - **macOS**:

       ```bash
       brew install fftw
       ```

     - **Linux**:

       ```bash
       sudo apt-get install libfftw3-dev
       ```

     - **Windows**:

       - Download precompiled binaries from the [FFTW website](http://www.fftw.org/install/windows.html).
       - Follow the installation instructions provided.

3. **Build the Application**

   - **Using Makefile**:

     ```bash
     make
     ```

   - **Manual Compilation**:

     ```bash
     gcc -o BragiBeats main.c audio_processing/audio_processing.c presentation/ui_renderer.c presentation/visualizers.c -lraylib -lm -lpthread -ldl -lfftw3
     ```

     *(Adjust the libraries linked based on your system and whether you're using FFTW.)*

---

## Usage

1. **Run the Application**

   ```bash
   ./BragiBeats
   ```

2. **Add Music**

   - Place your audio files (`.mp3`, `.wav`) into the `./media` directory.
   - Use drag-and-drop to add songs directly to the queue while the application is running.

3. **Playback Controls**

   - **Play/Pause**: Click the **Play/Pause** button or press `Space`.
   - **Skip Forward**: Click `>>` or press the `Right Arrow` key.
   - **Skip Backward**: Click `<<` or press the `Left Arrow` key.

4. **Visualizer Selection**

   - Click the **Visualizers** button on the title bar.
   - Choose from the list of available visualizers.

5. **Test Mode**

   - Toggle **Testing** mode by clicking the **Testing** button.
   - Select **Test Signals** to choose a specific test signal.

6. **Song Queue**

   - View the current playlist on the left panel.
   - Click on any song to play it immediately.

7. **Progress Bar**

   - Click or drag on the progress bar to seek within the current song.

---

## Customization

### Visualizers

Bragi Beats comes with multiple built-in visualizers. You can select them via the **Visualizers** button. Each visualizer offers a unique representation of the audio.

- **Adding New Visualizers**:

  - Create a new function in `visualizers.c` following the existing examples.
  - Update `visualizers.h` to include your new function.
  - Modify the UI rendering code to add your visualizer to the selection list.

### Test Signals

Test signals are useful for analyzing specific frequencies and behaviors.

- **Available Test Signals**:

  - **Sine Wave**
  - **Multi-Sine**
  - **Chirp**
  - **White Noise**

- **Selecting Test Signals**:

  - Activate **Testing** mode.
  - Click on **Test Signals** and choose the desired signal.

---

## Development

### Switching FFT Algorithms

Bragi Beats allows you to switch between the custom FFT implementation and a professional library (like FFTW) for comparison.

- **Toggle FFT Algorithm**:

  - Press the `F` key during runtime to switch between algorithms.
  - The current algorithm is displayed in the status message.

- **Implementation Details**:

  - The `fft` function in `audio_processing.c` checks the `currentFFTAlgorithm` variable to decide which implementation to use.

- **Extending to Other Libraries**:

  - You can integrate other FFT libraries by following the pattern established with FFTW.

### Performance Measurement

- **Timing Execution**:

  - The application measures and prints the execution time of the FFT computations.
  - This information can be used to compare the performance of different algorithms.

- **Optimization Tips**:

  - Ensure proper memory management to prevent leaks.
  - Use compiler optimization flags (e.g., `-O2`, `-O3`).
  - Consider parallel processing or SIMD instructions for heavy computations.

---

## License

Bragi Beats is licensed under the **MIT License**.

You are free to use, modify, and distribute this software. See the [LICENSE](LICENSE) file for details.

---
