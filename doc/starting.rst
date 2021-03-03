.. role:: cc(code)
   :language: c

Starting
========

There is a wrapper function for creating each port type, :c:func:`start_port`.
It accepts a mode argument and calls 4 lower-level wrapper functions.

Types of MIDI ports
-------------------

There are virtual and non-virtual MIDI ports.

Think about virtual MIDI input and ouptut in terms of endpoints you connect to.
"Normal" or "non-virtual" MIDI ports connect to those.

`Client—server model <https://en.wikipedia.org/wiki/Client%E2%80%93server_model>`_, where a client [#client_term]_ connects to a server
is a nice analogy, as well. In this analogy, a server is a "virtual port"
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

.. rubric:: Footnotes

.. [#client_term] Alsa has its own
                  `client term <https://www.alsa-project.org/alsa-doc/alsa-lib/seq.html#seq_client>`_ ,
                  that has a different meaning.
