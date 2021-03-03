[![](https://readthedocs.org/projects/rmr/badge/?version=latest&style=flat)](https://rmr.readthedocs.io/)

# RMR: RtMIDI, reduced

Hello and welcome to RMR's readme.

This is a simplified rewrite of RtMIDI's Alsa part: C++ → C + GLib library.
API calls and struct members **are different** from RtMIDI.

This is my personal, derivative project, although I'd be happy if you discuss it with me in [issues](https://github.com/6r1d/rmr/issues).

**Warning**: I might change API a lot, this project **is not stable**. Use RMR at your own risk.

## Documentation

[Latest documentation](https://rmr.readthedocs.io/en/latest/index.html) is available on [Read the Docs](https://readthedocs.org/).

## Motivation

* I want to use RMR on Linux embedded devices and learn more about MIDI and [Alsa](https://www.alsa-project.org/wiki/Main_Page) while changing the code
* I want to know the library internals

## Changes

* C++ → C
* use [GLib](https://developer.gnome.org/glib/stable) [asynchronous queues](https://developer.gnome.org/glib/stable/glib-Asynchronous-Queues.html) and [arrays](https://developer.gnome.org/glib/stable/glib-Arrays.html) when needed (mostly input code: arrays for variably sized SysEx messages and queues for inter-thread message passing)
* removed OOP parts and use structs
* changed OOP methods to simple functions
* use of `GNU make` for examples
* documentation using Sphinx and Hawkmoth

## Future plans

### Jack

One day, I might try and rewrite [JACK](https://jackaudio.org/) API and add it to RMR, I like using it for audio, but I don't use it for MIDI yet.

## Other great RtMIDI forks

There are other RtMIDI forks like [RtMIDI17](https://github.com/jcelerier/RtMidi17) for [C++17](https://en.wikipedia.org/wiki/C%2B%2B17).

