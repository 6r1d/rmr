.. role:: cc(code)
   :language: c

Starting
========

There is a wrapper function for creating each port type, :c:func:`start_port`.
It accepts a mode argument and calls 4 lower-level wrapper functions.

Types of MIDI ports
-------------------

There are "normal" and virtual MIDI ports.

Think about virtual MIDI input and ouptut in terms of endpoints you connect to.
"Normal" MIDI ports connect to those.

Client connecting to a server is a good analogy, too. In this analogy, a server is a "virtual port"
and a client is "just a port".

Calling wrappers
----------------

+----------------+--------------------+------------------------------------------------+
| MIDI port type | Use                | Init function                                  |
+================+====================+================================================+
| Input          | Connect to         | :cc:`start_port(&amidi_data, MP_IN);`          |
+----------------+--------------------+------------------------------------------------+
| Output         | Connect to         | :cc:`start_port(&amidi_data, MP_OUT);`         |
+----------------+--------------------+------------------------------------------------+
| Virtual input  | Create an endpoint | :cc:`start_port(&amidi_data, MP_VIRTUAL_IN);`  |
+----------------+--------------------+------------------------------------------------+
| Virtual output | Create an endpoint | :cc:`start_port(&amidi_data, MP_VIRTUAL_OUT);` |
+----------------+--------------------+------------------------------------------------+

Installation
------------

Just clone the repo and include the library as the examples show.

Requirements
------------

* Alsa — should be already installed in your Linux distro
* GNU Make — generally available in your Linux distro
* glib-2.0 — **libglib2.0-dev** in Ubuntu
