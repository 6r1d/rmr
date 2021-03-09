Examples
========

This page shows several examples of using RMR.

Picking a mode
--------------

If you are writing a code that other programs will find and connect to, create a **virtual** port.

Otherwise, connect to software or devices using normal **input** or **output** ports.

Building
--------

Open each directory, type "make" and it should be enough to get an executable file in that directory.

"Virtual input" is compatible with output, run "virtual input" first.

"Virtual output" is compatible with input, run "virtual output" first.


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

.. literalinclude:: ../examples/input_bytes/input.c
   :language: c
   :linenos:

Output
------

.. literalinclude:: ../examples/output/output.c
   :language: c
   :linenos:

Using :c:func:`get_full_port_name`
----------------------------------

This example finds and identifies an input or output port.

If you run a "virtual output" example and expected_port_type variable is set as :c:member:`mp_type_t.MP_VIRTUAL_OUT`, it will display "rmr virtual output:rmr virtual output port 128:0".

If you run a "virtual input" example and expected_port_type variable is set as :c:member:`mp_type_t.MP_VIRTUAL_IN`, it will display "rmr virtual input:rmr 128:0".

While this output format might look unusual, it provides info about two Alsa containers: "client info" and "port info", in that order.

.. literalinclude:: ../examples/get_full_port_name/get_full_port_name.c
   :language: c
   :linenos:
