#include <stdio.h>
#include <stdbool.h>
// Keeps process running until Ctrl-C is pressed.
// Contains a SIGINT handler and keep_process_running variable.
#include "util/exit_handling.h"
#include "util/output_handling.h"
#include "util/midi_parsing.h"
// Main RMR header file
#include "midi/midi_handling.h"

Alsa_MIDI_data * data;
MIDI_in_data * input_data;

MIDI_message * msg;
error_message * err_msg;

RMR_Port_config * port_config;

void message_handler(
    double timestamp,
    unsigned char * buf,
    long count,
    void * user_data
) {
    print_midi_msg_buf(buf, count);
}

int main() {
    // Allocate a MIDI_in_data instance, assign a
    // MIDI message queue and an error queue
    prepare_input_data_with_queues(&input_data);

    // Create a port configuration with default values
    setup_port_config(&port_config, MP_VIRTUAL_IN);
    // Start a port with a provided configruation
    start_port(&data, port_config);

    // Assign amidi_data to input_data instance
    assign_midi_data(input_data, data);

    // Open a new port with a pre-set name
    open_virtual_port(data, "rmr", input_data);

    // Prepare to handle input through a callback
    set_MIDI_in_callback(input_data, message_handler, NULL);

    // Don't exit until Ctrl-C is pressed;
    // Look up "output_handling.h"
    keep_process_running = 1;

    // Add a SIGINT handler to set keep_process_running to 0
    // so the program can exit
    signal(SIGINT, sigint_handler);

    // Run until SIGINT is received
    while (keep_process_running) {
        while (g_async_queue_length_unlocked(input_data->error_async_queue)) {
            // Read an error message from an error queue,
            // simply deallocate it for now
            err_msg = g_async_queue_try_pop(input_data->midi_async_queue);
            if (err_msg != NULL) free_error_message(err_msg);
        }
    }

    // Close a MIDI input port, shutdown the input thread,
    // do cleanup
    destroy_midi_input(data, input_data);

    // Destroy a port configuration
    destroy_port_config(port_config);

    // Exit without an error
    return 0;
}
