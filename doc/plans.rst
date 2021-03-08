Plans
=====

There are things I want to change, but issues are already full with random stuff I'll need to track.
This document exists to handle things gradually, not create issues when I solve other issues.
(It seems I will always create more than I solve.)

**P.S.**: this doesn't seem like a good way to continue, because now there's a "clutter 1" and "clutter 2", but I'll let time to show how it goes.

------------

Terminology
-----------

Terms I want to change
^^^^^^^^^^^^^^^^^^^^^^

Check if "port_config->virtual_seq_name" is a term that makes sense, rename if it doesn't.

Callback and queues
-------------------

Error queue in callback examples
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When I finished a callback example, closing issue #14, I noticed there's still a queue for errors that happened in input thread.
I am thinking about a different method of error propagation. There are also disconnections, which aren't exactly errors.
I think I can replace error queue with callback calls easily.
In which thread the callback runs, though?
I also feel like I'll have to redesign "serr" function to work well with a new "error" or "status callback" mechanism.

Implementation details
----------------------

Port capabilities while retrieving a port name
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

RtMIDI has two Alsa `getPortName` methods, I have one `get_full_port_name` function.

Port capabilities in RtMIDI might've been inverted, and I think that I have inverted them back, properly. I am probably wrong, but I debugged it and it works for some reason.

.. code:: c

  if (in) port_mode = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;
  else    port_mode = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;

"SND_SEQ_PORT_CAP_WRITE" is defined as "writable to this port", too.

Does it mean RtMIDI's "getPortName" did not work in a recent release, or am I wrong somewhere?..

Architectural changes
---------------------

Default amount of channels?
^^^^^^^^^^^^^^^^^^^^^^^^^^^

snd_seq_port_info_set_midi_channels accepts 16 channels. It sounds like a magic constant I'll have to fix, but I am not sure about that.
If that is correct, a define seems enough, but moving amount of channels into a port configurator sounds even better.

MIDI2 handling
^^^^^^^^^^^^^^

MIDI2 support doesn't seem to be globally implemented.
I receive mixed news: some claim to have finished MIDI2 compatible synths, some claim MIDI2 is not supported by Windows, etc.
For now I'll collect information and will try to prepare.

* https://github.com/jackaudio/jack2/issues/535
* https://github.com/atsushieno/cmidi2

Memory management
-----------------

Check all "malloc", "calloc", "alloca" calls are cleaned up after.

Self-reviews
------------

Read the header code through at least 4 times.
I'm sure I missed something, be it apidoc or other things.

Current times: 1.
