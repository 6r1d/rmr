#include <stdbool.h>
#include "util/exit_handling.h"
#include "midi/midi_handling.h"

Alsa_MIDI_data * data;
MIDI_in_data * input_data;

MIDI_port * current_midi_port;

MIDI_message * msg;
error_message * err_msg;

void print_midi_msg_buf(unsigned char * buf, long count) {
    unsigned int byte_idx;
    for (byte_idx = 0; byte_idx < count; byte_idx++) {
        printf("%d ", buf[byte_idx]);
    }
    printf("\n");
    fflush( stdout );
}

void print_midi_message_pointer(MIDI_message * msg) {
    printf("PTR: %p\n", (void *) msg);
    fflush( stdout );
}

int main() {
    prepare_input_data_with_queues(&input_data);
    start_port(&data, MP_IN);

    init_midi_port(&current_midi_port);

    // Assigns amidi_data to input_data instance
    assign_midi_data(input_data, data);

    // Count the MIDI ports,
    // open if a port containing "Synth" is available
    if (find_midi_port(data, current_midi_port, "Bonsai", false) > 0) {
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
                print_midi_message_pointer(msg);
                free_midi_message(msg);
            }
        }
        while (g_async_queue_length_unlocked(input_data->error_async_queue)) {
            err_msg = g_async_queue_try_pop(input_data->midi_async_queue);
            if (err_msg != NULL) free_error_message(err_msg);
        }
    }

    destroy_midi_input(data, input_data);

    // Exit without an error
    return 0;
}
