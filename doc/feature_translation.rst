RtMIDI feature translation
==========================

Queue messages as bytes
^^^^^^^^^^^^^^^^^^^^^^^

`RtMIDI <https://www.music.mcgill.ca/~gary/rtmidi/>`_ translates
all `ALSA <https://www.alsa-project.org/alsa-doc/alsa-lib/>`_ messages to bytes. [#byte_use_reason]_

It is left the same way, although data types might be changed to ensure bytes are always 8-bit (something like :c:type:`uint8_t`).

Rejoining message chunks
^^^^^^^^^^^^^^^^^^^^^^^^

The `ALSA <https://www.alsa-project.org/alsa-doc/alsa-lib/>`_ sequencer has
a maximum buffer size for MIDI sysex events of 256 bytes.

If a device sends sysex messages larger than this, they are segmented into 256 byte chunks.

`RtMIDI <https://www.music.mcgill.ca/~gary/rtmidi/>`_ rejoins 256-byte SysEx
message chunks to a single bytearray.

Adding timestamps to MIDI messages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The initial source contains two ways to do that.

First one uses the system time.

.. code-block:: c

  (void)gettimeofday(&tv, (struct timezone *)NULL);
  time = (tv.tv_sec * 1000000) + tv.tv_usec;

Second one uses the ALSA sequencer event time data and was implemented
by `Pedro Lopez-Cabanillas <https://github.com/pedrolcl>`_. [#libc_elapsed]_

The second one was commented quite a bit and it can be found in a current source code.

init_seq client name setting
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Uses an internal function and different ordering, but it doesn't seem to influence anything and this doc section is about to be removed.

**Before**:

.. code-block:: c

    snd_seq_set_client_name(seq, client_name);
    amidi_data->seq = seq;

**Now**:

.. code-block:: c

    amidi_data->seq = seq;
    set_client_name(amidi_data, client_name);


.. rubric:: Footnotes

.. [#byte_use_reason] *I'm not sure if it helps to rejoin SysEx messages or it is done as a way to unify output for Alsa, Jack, etc.*

.. [#libc_elapsed] LibC manual on `getting elapsed time <https://www.gnu.org/software/libc/manual/html_node/Calculating-Elapsed-Time.html>`_.
