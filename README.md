Splash, a multi-projector video-mapping software
================================================

[![GPLv3 license](https://img.shields.io/badge/License-GPLv3-blue.svg)](http://perso.crans.org/besson/LICENSE.html)
[![pipeline status](https://gitlab.com/sat-metalab/splash/badges/develop/pipeline.svg)](https://gitlab.com/sat-metalab/splash/commits/develop)
[![coverage report](https://gitlab.com/sat-metalab/splash/badges/develop/coverage.svg)](https://gitlab.com/sat-metalab/splash/commits/develop)

For a more complete documentation, go visit the [official website](https://sat-metalab.gitlab.io/splash).

## Table of Contents

[Introduction](#introduction)

[Installation](#installation)

[Code contribution](#code-contribution)

[Going forward](#going-forward)


## Introduction

### About
Splash is a free (as in GPL) modular mapping software. Provided that the user creates a 3D model with UV mapping of the projection surface, Splash will take care of calibrating the videoprojectors (intrinsic and extrinsic parameters, blending and color), and feed them with the input video sources. Splash can handle multiple inputs, mapped on multiple 3D models, and has been tested with up to eight outputs on two graphic cards. It currently runs on a single computer but support for multiple computers is planned.

Although Splash was primarily targeted toward fulldome mapping and has been extensively tested in this context, it can be used for virtually any surface provided that a 3D model of the geometry is available. Multiple fulldomes have been mapped, either by the authors of this software (two small dome (3m wide) with 4 projectors, a big one (20m wide) with 8 projectors) or by other teams. It has also been tested sucessfully as a more regular video-mapping software to project on buildings, or [onto moving objects](https://vimeo.com/268028595).

Regarding performances, our tests show that Splash can handle flawlessly a 3072x3072@60Hz live video input, or a 4096x4096@60Hz video while outputting to eight outputs (through two graphic cards) with a high end cpu and the [HapQ](http://vdmx.vidvox.net/blog/hap) video codec (on a SSD as this codec needs a very high bandwidth). Due to its architecture, higher resolutions are more likely to run smoothly when a single graphic card is used, although nothing higher than 4096x4096@60Hz has been tested yet (well, we tested 6144x6144@60Hz but the drive throughput was not sufficient to sustain the video bitrate).

Splash can read videos from various sources amoung which video files (most common format and Hap variations), video input (such as video cameras and capture cards), and Shmdata (a shared memory library used to make softwares from the SAT Metalab communicate between each others). An addon for Blender is included which allows for exporting draft configurations and update in real-time the meshes. It also handles automatically a few things:
- semi automatic geometric calibration of the video-projectors,
- automatic calibration of the blending between them,
- experimental automatic colorimetric calibration (with a gPhoto compatible camera)

### Licenses
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program uses external libraries, some of them being bundled in the source code repository (directly or as submodules). They are located in `external`, and are not necessarily licensed under GPLv3. Please refer to the respective licenses for details.

### Authors
See [AUTHORS.md](docs/Authors.md)

### Project URL
This project can be found either on [its official website](https://sat-metalab.gitlab.io/splash), on the [SAT Metalab repository](https://gitlab.com/sat-metalab/splash) or on [Github](https://github.com/paperManu/splash).

### Sponsors
This project is made possible thanks to the [Society for Arts and Technologies](http://www.sat.qc.ca) (also known as SAT).


## Installation

### Dependencies
Splash relies on a few libraries to get the job done. The mandatory libraries are:

- External dependencies:
  - [OpenGL](http://opengl.org), which should be installed by the graphic driver,
  - [GSL](http://gnu.org/software/gsl) (GNU Scientific Library) to compute calibration,
- External dependencies bundled as submodules:
  - [FFmpeg](http://ffmpeg.org/) to read video files,
  - [GLFW](http://glfw.org) to handle the GL context creation,
  - [GLM](http://glm.g-truc.net) to ease matrix manipulation,
  - [Snappy](https://code.google.com/p/snappy/) to handle Hap codec decompression,
  - [ZMQ](http://zeromq.org) to communicate between the various process involved in a Splash session,
  - [cppzmq](https://github.com/zeromq/cppzmq.git) for its C++ bindings of ZMQ
  - [JsonCpp](http://jsoncpp.sourceforge.net) to load and save the configuration,
- Dependencies built at compile-time from submodules:
  - [doctest](https://github.com/onqtam/doctest/) to do some unit testing,
  - [ImGui](https://github.com/ocornut/imgui) to draw the GUI,
  - [stb_image](https://github.com/nothings/stb) to read images.

Some other libraries are optional:

- External dependencies:
  - [libshmdata](http://gitlab.com/sat-metalab/shmdata) to read video flows from a shared memory,
  - [portaudio](http://portaudio.com/) to read and output audio,
  - [Python](https://python.org) for scripting capabilities,
  - [GPhoto](http://gphoto.sourceforge.net/) to use a camera for color calibration.
- Dependencies built at compile-time from submodules:
  - [libltc](http://x42.github.io/libltc/) to read timecodes from an audio input,

Also, the [Roboto](https://www.fontsquirrel.com/fonts/roboto) font and the [DSEG font family](https://github.com/keshikan/DSEG) are used and distributed under their respective open source licenses.

By default Splash is built and linked against the libraries included as submodules, but it is possible to force it to use the libraries installed on the system. This is described in the next section.

### Installation

#### Linux

Splash can be installed from a pre-built package, or compiled by hand. Newcomers are advised to try the packaged version first, and try to compile it if necessary only.

##### Packages

The easiest way to install and test Splash is by using the [Flatpak](https://flatpak.org) archive, which is compatible with most Linux distributions. Splash is available on [Flathub](https://flathub.org/apps/details/com.gitlab.sat_metalab.Splash), and can be installed as follows on Ubuntu:

```bash
sudo apt install flatpak
sudo flatpak install flathub org.freedesktop.Platform//1.6
sudo flatpak install flathub com.gitlab.sat_metalab.Splash
```

Splash should now be accessible from your application menu (this may require to logout / log back in), or it can be run directly from the terminal as well:

```bash
flatpak run com.gitlab.sat_metalab.Splash
```

A known limitation of the Flatpak package is that it has no access to Jack.

The current release of Splash is also packaged for Ubuntu (version 20.04) and derived. This is done through a Debian archive available on the [tags page](https://gitlab.com/sat-metalab/splash/tags), and install it with :

```bash
sudo apt install <download path>/splash-<version>-Linux.deb
```

##### Building

You can also compile Splash by hand, especially if you are curious about its internals or want to tinker with the code (or even, who knows, contribute!). Note that although what follows compiles the develop branch, it is more likely to contain bugs alongside new features / optimizations so if you experience crash you can try with the master branch.

The packages necessary to compile Splash are the following:
- Ubuntu 20.04 and derivatives:

```bash
sudo apt install build-essential git-core cmake libxrandr-dev libxi-dev \
    mesa-common-dev libgsl0-dev libatlas3-base libgphoto2-dev libz-dev \
    libxinerama-dev libxcursor-dev python3-dev yasm portaudio19-dev \
    python3-numpy libopencv-dev gcc-8 g++-8 libjsoncpp-dev libx264-dev \
    libx265-dev

# Non mandatory libraries needed to link against system libraries only
sudo apt install libglfw3-dev libglm-dev libavcodec-dev libavformat-dev \
    libavutil-dev libswscale-dev libsnappy-dev libzmq3-dev libzmqpp-dev
```

- Archlinux (not well maintained, please signal any issue):

```bash
pacman -Sy git cmake make gcc yasm pkgconfig libxi libxinerama libxrandr libxcursor jsoncpp \
    mesa glm gsl libgphoto2 python3 portaudio zip zlib x264 x265 opencv qt5-base vtk hdf5 glew
```

Once everything is installed, you can go on with building Splash. To build and link it against the bundled libraries:

```bash
git clone https://gitlab.com/sat-metalab/splash
cd splash
./make_deps.sh
mkdir -p build && cd build
cmake ..
make -j$(nproc) && sudo make install
```

Otherwise, to build Splash and link it against the system libraries:

```bash
git clone https://gitlab.com/sat-metalab/splash
cd splash
mkdir -p build && cd build
cmake -DUSE_SYSTEM_LIBS=ON ..
make -j$(nproc) && sudo make install
```

You can now try launching Splash:

```bash
splash --help
```

If you want to have access to realtime scheduling within Splash, you need to create a group "realtime", add yourself to it and set some limits:

```bash
sudo addgroup realtime
sudo adduser $USER realtime
sudo cp ./data/config/realtime.conf /etc/security/limits.d/
```

And if you want the logs to be written to /var/log/splash.log:

```bash
sudo adduser $USER syslog
```

Then log out and log back in.

If you want to specify some defaults values for the objects, you can set the environment variable SPLASH_DEFAULTS with the path to a file defining default values for given types. An example of such a file can be found in [data/config/splashrc](data/config/splashrc)

And that's it, you can move on the the [Walkthrough](https://sat-metalab.gitlab.io/splash/Walkthrough/) page.


## Code contribution

Contributions are welcome ! See [CONTRIBUTING.md](docs/Contributing.md) and [CODE_OF_CONDUCT.md](docs/Code_of_conduct.md) for details.


## Going forward

To learn how to configure and use Splash, the best resource is [its official website](https://sat-metalab.gitlab.io/splash).
