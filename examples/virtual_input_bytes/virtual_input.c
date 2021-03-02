#include <stdio.h>
#include <stdbool.h>
#include "util/exit_handling.h"
#include "util/midi_parsing.h"
#include "midi/midi_handling.h"

Alsa_MIDI_data * data;
MIDI_in_data * input_data;

MIDI_message * msg;
error_message * err_msg;

// TODO move somewhere
void print_midi_msg_buf(unsigned char * buf, long count) {
    long byte_id;
    for (byte_id = 0; byte_id < count; byte_id++) {
        printf("%02x ", buf[byte_id]);
    }
    printf(" | ");
    for (byte_id = 0; byte_id < count; byte_id++) {
        printf("%d ", buf[byte_id]);
    }
    printf("\n");
    fflush( stdout );
}

int main() {
    //
    prepare_input_data_with_queues(&input_data);
    // TODO look thru each required call
    start_port(&data, MP_VIRTUAL_IN);
    //
    assign_midi_data(input_data, data);
    // Open a new port with a pre-set name
    open_virtual_port(data, "Synth", input_data);

    keep_process_running = 1;

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

    // Exit without an error
    return 0;
}
