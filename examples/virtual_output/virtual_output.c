#include <stdio.h>
#include <stdbool.h>
#include <unistd.h> // usleep
#include <time.h>   // nanosleep
#include "midi/midi_handling.h"
#include "util/exit_handling.h"
#include "util/timing.h"
#include "test_data.h"

Alsa_MIDI_data * amidi_data;

// Send "note on" or "note off" signal
bool msg_mode = false;
// Last recorded time in milliseconds
double timer_msec_last;

int main() {
    // Record initial time to a timer
    timer_msec_last = millis();

    start_port(&amidi_data, MP_VIRTUAL_OUT);

    // Send out a series of MIDI messages.
    send_midi_message(amidi_data, MIDI_PROGRAM_CHANGE_MSG, 2);
    send_midi_message(amidi_data, MIDI_CONTROL_CHANGE_MSG, 3);

    // Add SIGINT handler
    signal(SIGINT, sigint_handler);
    keep_process_running = 1;

    // Generate notes until SIGINT
    while (keep_process_running) {
        if (millis() - timer_msec_last > 100.) {
            timer_msec_last = millis();
            if (msg_mode) send_midi_message(amidi_data, MIDI_NOTE_ON_MSG, 3);
            else          send_midi_message(amidi_data, MIDI_NOTE_OFF_MSG, 3);
            printf("mode: %d\n", msg_mode);
            msg_mode = !msg_mode;
            fflush(stdout);
            usleep(10);
        }
    }

    if (destroy_midi_output(amidi_data, NULL) != 0) slog("destructor", "destructor error");

    // Exit without an error
    return 0;
}
