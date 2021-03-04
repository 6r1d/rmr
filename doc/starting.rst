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

Calling a wrapper
-----------------

There is a single wrapper that is made of two parts: a port configurator (:c:func:`setup_port_config`)
and a port starter (:c:func:`start_port`).

The table below shows some init examples and there's more info in the examples section.

+----------------+--------------------+-----------------------------------------------------+
| MIDI port type | Use                | Init function                                       |
+================+====================+=====================================================+
| Input          | Connect to         | .. code-block:: c                                   |
|                |                    |                                                     |
|                |                    |    setup_port_config(&port_config, MP_IN);          |
|                |                    |    start_port(&amidi, port_config);                 |
+----------------+--------------------+-----------------------------------------------------+
| Output         | Connect to         | .. code-block:: c                                   |
|                |                    |                                                     |
|                |                    |    setup_port_config(&port_config, MP_OUT);         |
|                |                    |    start_port(&amidi, port_config);                 |
+----------------+--------------------+-----------------------------------------------------+
| Virtual input  | Create an endpoint | .. code-block:: c                                   |
|                |                    |                                                     |
|                |                    |    setup_port_config(&port_config, MP_VIRTUAL_IN);  |
|                |                    |    start_port(&amidi, port_config);                 |
+----------------+--------------------+-----------------------------------------------------+
| Virtual output | Create an endpoint | .. code-block:: c                                   |
|                |                    |                                                     |
|                |                    |    setup_port_config(&port_config, MP_VIRTUAL_OUT); |
|                |                    |    start_port(&amidi, port_config);                 |
+----------------+--------------------+-----------------------------------------------------+

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
                  that has a different meaning: an Alsa seq client that has one or more
                  ports as endpoints to communicate. Each port can be in a mode to read or write data
                  and have other options.
