Examples
========

This page shows several examples of using RMR.

Which mode to use?
------------------

If you are writing a code that other programs will find and connect to, create a **virtual** port.

Otherwise, connect to software or devices using normal **input** or **output** ports.

How to build the examples?
--------------------------

Open each directory, type "make" and it should be enough to get an executable file in that directory.

"Virtual input" is compatible with output, run "virtual input" first.

"Virtual output" is compatible with input, rin "virtual output" first.


Virtual input
-------------

.. literalinclude:: ../examples/virtual_input_bytes/virtual_input.c
   :language: c
   :linenos:

Virtual output
--------------

.. literalinclude:: ../examples/virtual_output/virtual_output.c
   :language: c
   :linenos:

Note: inconsistent intervals
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

I had inconsistent note intervals while making this example.

I've used **millis()** and **usleep(N)** calls.
It's possible to feed usleep values that make these intervals inconsistent.
Current intervals are **100**, but I'd have to play with this idea more.

.. code-block:: c

  // Returns the milliseconds since Epoch
  double millis() {
          struct timeval cur_time;
          gettimeofday(&cur_time, NULL);
          return (cur_time.tv_sec * 1000.0) + cur_time.tv_usec / 1000.0;
  }

Input
-----

.. literalinclude:: ../examples/input/input_bytes.c
   :language: c
   :linenos:

Output
------

.. literalinclude:: ../examples/output/output.c
   :language: c
   :linenos:

