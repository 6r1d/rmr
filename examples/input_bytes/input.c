#include <stdbool.h>
#include "util/exit_handling.h"
#include "util/output_handling.h"
#include "midi/midi_handling.h"

Alsa_MIDI_data * data;
MIDI_in_data * input_data;

MIDI_port * current_midi_port;

RMR_Port_config * port_config;

MIDI_message * msg;
error_message * err_msg;

int main() {
    prepare_input_data_with_queues(&input_data);

    // Create a port configuration with default values
    setup_port_config(&port_config, MP_IN);
    // Start a port with a provided configruation
    start_port(&data, port_config);

    init_midi_port(&current_midi_port);

    // Assigns amidi_data to input_data instance
    assign_midi_data(input_data, data);

    // Count the MIDI ports,
    // open if a port containing "Synth" is available
    if (find_midi_port(data, current_midi_port, "rmr", false) > 0) {
        print_midi_port(current_midi_port);
        open_port(true, data, current_midi_port->id, current_midi_port->port_info_name, input_data);
        keep_process_running = 1;
    }

    // Add SIGINT handler
    signal(SIGINT, sigint_handler);

    // Run until SIGINT is received
    while (keep_process_running) {
        while (g_async_queue_length_unlocked(input_data->midi_async_queue)) {
            msg = g_async_queue_try_pop(input_data->midi_async_queue);
            if (msg != NULL) {
                print_midi_msg_buf(msg->buf, msg->count);
                free_midi_message(msg);
            }
        }
        while (g_async_queue_length_unlocked(input_data->error_async_queue)) {
            err_msg = g_async_queue_try_pop(input_data->midi_async_queue);
            if (err_msg != NULL) free_error_message(err_msg);
        }
    }

    destroy_midi_input(data, input_data);

    // Destroy a port configuration
    destroy_port_config(port_config);

    // Exit without an error
    return 0;
}
