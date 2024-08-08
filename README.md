# Bragi Beats

Bragi Beats is an audio visualizer application. Music will be preprocessed with
an FFT algorithm. The data from the FFT algorithm will be rendered with the RayLib library in real time.

## Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Features](#features)
- [Screenshots](#screenshots)
- [Videos](#videos)
- [License](#license)

## Installation

1. Clone the Repository:
```sh
git clone https://github.com/aarongc43/bragi-beats.git
```

2. Navigate to the project directory:

```sh
cd bragi-beats/
```

3. Install dependencies and build the project:
```sh
make all
```

## Usage

1. Run the application:

```sh 
./bragibeats
```

2. Follow the on-screen instructions to visualize your audio files.

## Features

- Drag and Drop: Drag and drop audio files and they will autoplay.

- Queue: Every audio file that is added gets put into a queue.

- Playback Controls: Play/Pause, Skip Forward and Backwards, Select a specific
point on the playback timeline to skip to.

- Visualizers: There are several visualizers that are proof of concepts to show
  that the FFT algorithm is being used create the visualizations.

- Login: Currently there is a login feature that was supposed to use a database
  to for users to login and create accounts, but this is being removed soon and
  changed to load a library folder.

![Drag and Drop](/assets/drag_and_drop.png)
![Playing](/assets/playing.png)
![Library](/assets/library.png)

## Videos

Short video for demonstration of the app's functionality:

![Demo Video](/assets/playing_video.gif)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
