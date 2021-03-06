[![](https://img.shields.io/badge/docs-sphinx-blue.svg)](https://rmr.readthedocs.io/)
[![](https://readthedocs.org/projects/rmr/badge/?version=latest&style=flat)](https://rmr.readthedocs.io/)
![](https://img.shields.io/github/v/release/6r1d/rmr?include_prereleases?style=flat)
[![](https://img.shields.io/github/license/6r1d/rmr?style=flat)](https://github.com/6r1d/rmr/blob/main/LICENSE)
[![GitHub Issues](https://img.shields.io/github/issues/6r1d/rmr.svg)](https://github.com/6r1d/rmr/issues)
[![Percentage of issues still open](http://isitmaintained.com/badge/open/6r1d/rmr.svg)](http://isitmaintained.com/project/6r1d/rmr "Percentage of issues still open")
[![Average time to resolve an issue](https://isitmaintained.com/badge/resolution/6r1d/rmr.svg)](https://isitmaintained.com/project/6r1d/rmr "Average time to resolve an issue")

Hello and welcome to RMR's readme.

RMR or "RtMIDI, reduced" is a simplified rewrite of [RtMIDI](https://github.com/thestk/rtmidi)'s Alsa part: C++ → C + GLib library.
It allows to interface C programs using [MIDI](https://en.wikipedia.org/wiki/MIDI) standard.
There is no compatibility with API calls and struct members from [RtMIDI](https://github.com/thestk/rtmidi).

**Warning**: I might change API a lot, this project **is not stable**. Use RMR at your own risk.

## Documentation

[Latest documentation](https://rmr.readthedocs.io/en/latest/index.html) is available on [Read the Docs](https://readthedocs.org/).

## Motivation

This is my personal, derivative project. I want to use RMR on Linux embedded devices and learn more about [MIDI](https://en.wikipedia.org/wiki/MIDI) and [Alsa](https://www.alsa-project.org/wiki/Main_Page) while changing the code.
I also want to know the library internals well when I am using it so it can be as predictable for me as possible.

It will be great if you will discuss it with me in [issues](https://github.com/6r1d/rmr/issues), recommend code improvements, etc.

## Changes

* C++ → C
* use [GLib](https://developer.gnome.org/glib/stable) [asynchronous queues](https://developer.gnome.org/glib/stable/glib-Asynchronous-Queues.html) and [arrays](https://developer.gnome.org/glib/stable/glib-Arrays.html) when needed (mostly input code: arrays for variably sized SysEx messages and queues for inter-thread message passing)
* removed [OOP](https://en.wikipedia.org/wiki/Object-oriented_programming) parts and use structs to reduce overhead
* changed [OOP](https://en.wikipedia.org/wiki/Object-oriented_programming) methods to simple functions to reduce overhead
* currently uses `do {} while (0);` for error handling in many cases
* use of `GNU make` for examples and tests
* documentation using Sphinx and Hawkmoth

## Future plans

### Jack

One day, I might try and rewrite [JACK](https://jackaudio.org/) API and add it to RMR, I like using it for audio, but I don't use it for MIDI yet.

## Legal and ethical

The RMR is MIT-licensed.
If you create a pull request, it will also be very nice of you to add a PR for the original RtMIDI repo and I'm ready to help with that.
Initially, I wanted it to have the same non-binding license clause, but that way, GitHub can not detect the license.
(Please inform me in the issues if there's anything wrong with it.)

## Other great [RtMIDI](https://github.com/thestk/rtmidi) forks

* [RtMIDI17](https://github.com/jcelerier/RtMidi17): [C++17](https://en.wikipedia.org/wiki/C%2B%2B17)
* [libremidi](https://github.com/jcelerier/libremidi): [C++17](https://en.wikipedia.org/wiki/C%2B%2B17), based on RtMIDI and [ModernMIDI](https://github.com/ddiakopoulos/modern-midi), [should handle](https://github.com/thestk/rtmidi/issues/214#issuecomment-792327899) large SysEx messages better
