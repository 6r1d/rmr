RMR: RtMIDI, reduced
====================

Hello and welcome to RMR documentation.

RMR is a rewrite of `RtMIDI <https://github.com/thestk/rtmidi>`_ project's `ALSA <https://www.alsa-project.org/wiki/Main_Page>`_ [#jack]_ part from C++ to C with `GLib <https://developer.gnome.org/glib/>`_ (it uses both `GArray <https://developer.gnome.org/glib/stable/glib-Arrays.html>`_ and `GAsyncQueue <https://developer.gnome.org/glib/stable/glib-Asynchronous-Queues.html>`_ for input). It is made as a personal experiment for use on embedded devices.

It is using the `Sphinx <https://www.sphinx-doc.org>`_ documentation generator with `Hawkmoth <https://hawkmoth.readthedocs.io/en/latest/index.html>`_ extension.

Contents
================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   self
   starting
   examples
   terminology
   architecture
   api/api
   errors
   feature_translation
   rewrite_changes

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

.. rubric:: Footnotes

.. [#jack] I will probably rewrite Jack support, as well, but I don't have access and motivation for rewriting Mac / Windows parts.
