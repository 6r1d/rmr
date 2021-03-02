#include <stdio.h>
#include <stdbool.h>
#include "util/exit_handling.h"
#include "util/midi_parsing.h"
#include "midi/midi_handling.h"

Alsa_MIDI_data * data;
MIDI_in_data * input_data;

MIDI_message * msg;
error_message * err_msg;

unsigned char MIDI_MSG_NOTE_OFF = 0x80;
unsigned char MIDI_MSG_NOTE_ON = 0x90;
unsigned char MIDI_MSG_CONT_CTR = 0xB0;

void print_midi_msg_buf(unsigned char * buf, long count) {
    // unsigned int byte_idx;
    if (buf[0] == MIDI_MSG_NOTE_ON) {
        printf("Note On");
        printf(" n %d ", buf[1]); // note
        printf(" %f ", note_to_freq(buf[1]));
        printf(" v %02x ", buf[2]); // velocity
    }
    else if (buf[0] == MIDI_MSG_NOTE_OFF) {
        printf("Note Off");
        printf(" n %d ", buf[1]); // note
        printf(" z?%02x ", buf[2]); // zero
    }
    else if (buf[0] == MIDI_MSG_CONT_CTR) {
        printf("Continuous controller");
        printf(" a %d ", buf[1]); // addr
        printf(" v %d ", buf[2]); // value
    }
    else {
        for (long i = 0; i < count; i++) {
            printf("%02x(%d) ", buf[i], buf[i]);
        }
    }
    printf(" ");
    printf("\n");
    fflush( stdout );
}



int main() {
    //
    prepare_input_data_with_queues(&input_data);
    //
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
