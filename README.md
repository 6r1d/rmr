# RMR: RtMIDI, reduced

Hello. This is a "reduced" rewrite of RtMIDI's Alsa part, from C++ to pure C with GLib.
API calls and struct members are very different from RtMIDI and will change until things settle.

This is my personal, derivative project.

**I might change API a lot, project is not stable. Use RMR at your own risk.**
(But I'll be happy to discuss RMR with you in issues!)

There are many RtMIDI forks like [RtMIDI17](https://github.com/jcelerier/RtMidi17) for [C++17](https://en.wikipedia.org/wiki/C%2B%2B17).

## Motivation

* I want to use RMR on Linux embedded devices and learn more about MIDI and [Alsa](https://www.alsa-project.org/wiki/Main_Page) while changing the code
* I want to know the library internals

Compatibility with RTMidi is not my goal. Instead, I:

* remove OOP parts and use structs
* break down complex function calls to simple ones
* use only simple `GNU make` for examples
* use [GLib](https://developer.gnome.org/glib/stable) types when simple C tools are not enough for me
* try to document it all using Sphinx and Hawkmoth

## Future plans

### Jack

One day, I might try and rewrite [JACK](https://jackaudio.org/) API and add it to RMR, I like using it for audio, but I don't use it for MIDI yet.
