# PCManFm plugin to show audio/video file info in the statusbar using FFmpeg libraries

## Overview

This plugin shows a number of technical information about audio and video files (MP3, AVI, MKV...). A typical description has the following structure:
```
[<duration>][ | <video codec>[, <bitrate>][, <resolution>][, <frame rate>]][ | <audio codec>[, <bitrate>][, <sampling rate>][, <channels>]] 
```
An actual example is:
```
[3:42] | h264, 278 kbps, 640x630, 29,97 fps | aac, 95 kbps, 44 khz, stereo
```
Fields which can't be detected are omitted. If a file contains no audio or no video, the corresponding block is omitted. 

## Installation

First install dependencies. For debian-based systems:
```
sudo apt-get install libtool-bin libfm-dev libavcodec-dev libavformat-dev libavutil-dev
```
Then compile and install:
```
make
sudo make install
```
The plugin is installed in /usr/lib/x86_64-linux-gnu/pcmanfm, which is fine for 64-bit systems. For 32-bit, change x86_64 to i386 in the Makefile.

### Contacts
<livanh@bulletmail.org>
