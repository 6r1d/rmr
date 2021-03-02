Terminology
===========

PPQ / PPQN / TPQN value
-----------------------

Ticks are regular units of time a computer or hardware MIDI device uses for measuring time steps.

Pulses per quarter note, ticks per quarter note or PPQ defines **the base resolution of the ticks**,
and it indicates **the number of divisions a quarter note has been split into** (according to Sweetwater's "What is PPQN" page).
Thus, PPQ value of zero is invalid.

It is the smallest unit of time used for sequencing note and automation events.
PPQ is one of the two values Alsa uses to specify the tempo (another one is MIDI tempo).
PPQ cannot be changed while the Alsa queue is running. It must be set before the queue is started.
(For this library, it means that PPQ value stays while a port is open.)

According to Wikipedia, "modern computer-based MIDI sequencers designed to capture more nuance may use 960 PPQN and beyond".

Typical PPQ values: 24, 48, 72, 96, 120, 144, 168, 192, 216, 240, 288, 360, 384, 436, 480, 768, 960, 972, 1024.

MIDI Tempo and BPM
------------------

We often hear about "beats per minute" or "BPM" in computer music.

BPM itself means "the amount of quarter notes in every minute".
Sometimes, BPM values are not strict enough and we might need more control.

For that reason, Alsa and RMR do not rely on "BPM" itself.
Instead, Alsa is using so-called "MIDI tempo" and RMR provides an interface to configure it.
(MIDI tempo is one of the two values Alsa uses to specify the tempo, another one is PPQ.)

MIDI tempo is not "the amount of quarter notes in every minute", but "time in microseconds per quarter note", so it defines the beat tempo in microseconds.
Increasing MIDI tempo will increase the length of a tick, so it will make playback slower.

+------------------+-----------------------------------------------+-------------------+
| Name             | Description                                   | Example value     |
+==================+===============================================+===================+
| :math:`T`        | * MIDI tempo                                  | * unknown         |
|                  | * a value we are trying to find               | * calculated to   |
|                  | * time in microseconds per quarter note       |   500000          |
+------------------+-----------------------------------------------+-------------------+
| :math:`BPM`      | * Beats per minute                            | 120 bpm           |
|                  | * the amount of quarter notes in every minute |                   |
+------------------+-----------------------------------------------+-------------------+
| :math:`min_{ms}` | A minute in microseconds                      | 60 s * 1000000 ms |
+------------------+-----------------------------------------------+-------------------+

We calculate a MIDI tempo by dividing a minute in microseconds by an amount of BPM:

:math:`T = \frac{min_{ms}}{BPM}`

Let us assume our BPM value is 120 and substitute a minute in microseconds to calculate an exact value.

:math:`T = \frac{min_{ms}}{120} = \frac{60 \cdot 1000000}{120} = 500000`

* ALSA project - the C library reference - `Sequencer interface <https://www.alsa-project.org/alsa-doc/alsa-lib/seq.html>`_ - setting queue tempo is the most interesting
* `MIDI Technical Fanatic's Brainwashing Center <http://midi.teragonaudio.com/>`_ — `MIDI file format: tempo and timebase <http://midi.teragonaudio.com/tech/midifile/ppqn.htm>`_

Tempo in MIDI files
-------------------

MIDI files use both MIDI tempo and PPQ value to specify the tempo.
