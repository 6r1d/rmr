/*
 * Functions to help make sense of MIDI bytes
 */

#include <math.h>
#include <stdio.h>

/*
void print_midi_msg(MIDI_message * msg) {
    // Define variables for storing a byte and a byte index
    int k;
    unsigned char v;
    for (k=0; k<msg->bytes->len; k++) {
        if (k > (msg->bytes->len - 1)) {
            printf("-");
        } else {
            v = g_array_index(msg->bytes, unsigned char, k);
            printf("%d ", v);
        }
    }
    printf("\n");
}
*/

/*
 * Utility functions
 */

float note_to_freq(int note) {
    // Frequency of A note,
    // coomon value called Stuttgart pitch is 440 Hz
    int A_freq = 440;
    return (A_freq / 32) * pow(2, ((note - 9) / 12));
}
