#include <stdio.h>
#include <stdbool.h>
// Needed for usleep
#include <unistd.h>
// Needed for nanosleep
#include <time.h>
// Main RMR header file
#include "midi/midi_handling.h"
// Keeps process running until Ctrl-C is pressed.
// Contains a SIGINT handler and keep_process_running variable.
#include "util/exit_handling.h"
#include "util/timing.h"
#include "test_data.h"

Alsa_MIDI_data * data;
MIDI_port * current_midi_port;

RMR_Port_config * port_config;

// Send "note on" or "note off" signal
bool msg_mode = false;
// Last recorded time in milliseconds
double timer_msec_last;

int main() {
    // Record initial time to a timer
    timer_msec_last = millis();

    // Create a port configuration with default values
    setup_port_config(&port_config, MP_VIRTUAL_OUT);
    // Start a port with a provided configruation
    start_port(&data, port_config);

    // Allocate memory for a MIDI_port instance
    init_midi_port(&current_midi_port);

    // Count the MIDI ports,
    // open if a port containing a certain word is available
    if (find_midi_port(data, current_midi_port, MP_VIRTUAL_IN, "rmr") > 0) {
        print_midi_port(current_midi_port);
        open_port(MP_OUT, current_midi_port->id, current_midi_port->port_info_name, data, NULL);
        // Don't exit until Ctrl-C is pressed;
        // Look up "output_handling.h"
        keep_process_running = 1;
    }

    // Send out a series of MIDI messages.
    send_midi_message(data, MIDI_PROGRAM_CHANGE_MSG, 2);
    send_midi_message(data, MIDI_CONTROL_CHANGE_MSG, 3);

    // Add a SIGINT handler to set keep_process_running to 0
    // so the program can exit
    signal(SIGINT, sigint_handler);

    // Run until SIGINT is received
    while (keep_process_running) {
        if (millis() - timer_msec_last > 100.) {
            timer_msec_last = millis();
            // Send a Note On message
            if (msg_mode) send_midi_message(data, MIDI_NOTE_ON_MSG, 3);
            // Send a Note Off message
            else          send_midi_message(data, MIDI_NOTE_OFF_MSG, 3);
            printf("mode: %d\n", msg_mode);
            msg_mode = !msg_mode;
            fflush(stdout);
            usleep(10);
        }
    }

    // Destroy a MIDI output port:
    // close a port connection and perform a cleanup.
    if (destroy_midi_output(data, NULL) != 0) slog("destructor", "destructor error");

    // Destroy a port configuration
    destroy_port_config(port_config);

    // Exit without an error
    return 0;
}
