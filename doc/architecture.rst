Architecture
============

Approach to data
----------------

RMR is made to work well with in realtime, so:

* It doesn't use `OOP <https://en.wikipedia.org/wiki/Object-oriented_programming>`_ features and is written in C
* It is oriented on data (`structs <https://en.wikipedia.org/wiki/Struct_\(C_programming_language\)>`_ in particular) and operations possible with those

Queue use
---------

Both **input** and **virtual input** modes are using thread that communicates with the main code
using GLib's `Asynchronous Queue <https://developer.gnome.org/glib/stable/glib-Asynchronous-Queues.html>`_ mechanism
for both messages and errors.

* :c:member:`MIDI_in_data.midi_async_queue`
* :c:member:`MIDI_in_data.error_async_queue`

Messages are contained in a structure, :c:type:`MIDI_message`.

:c:member:`MIDI_message.buf` contains message bytes and :c:member:`MIDI_message.count` contains length of this byte array.

Each :c:type:`MIDI_message` also contains a :c:member:`MIDI_message.timestamp` member:
a double value, based on snd_seq_real_time_t contents and previous time values.

Double pointers
---------------

Often enough, you will be seeing pointers to pointers in code.
It is done for `allocating memory in a function <https://stackoverflow.com/questions/2838038/c-programming-malloc-inside-another-function>`_
or something close to that, usually.
