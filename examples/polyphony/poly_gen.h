/*
 * This header file provides a polyphony implementation.
 * Polyphony allows to play several notes simultaneously, each played note is called a voice.
 * A note stops at a certain moment and this is called "voice stealing".
 * It is based on Acratone with minimal changes.
 *
 * https://github.com/Saegor/acratone
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

// An amount of voices to use for polyphony
#define VOICES_CNT 16
// Maximum possible energy a voice has.
// High energy = long playing voice.
#define ENERGY_MAX 5000

// A structure for a polyphony voice,
// containing a number of samples
typedef struct {
    float phase;
    float freq;
    unsigned int midi_note;
    unsigned int energy;
    int hold : 1;
} poly_voice_t;

// Allocates an array of the polyphony voices
poly_voice_t * tab[VOICES_CNT] = {NULL};

// Converts a float phase value to a float sinusoid wave sample.
// Changes sine function period from 2 Pi to 1.
float phase_to_sin(float p) {
    return sinf(2 * M_PI * p);
}

// Remove a note from a voice tab.
// It doesn't matter if a voice is active.
void free_note(int id) {
    // Throw an error if a voice does not exist
    assert(tab[id] != NULL);
    // Free the memory
    free(tab[id]);
    // Clean the voice
    tab[id] = NULL;
}

// Clears a sound buffer.
void clear_buffer(int buffer_size, float * buffer_out) {
    for (int bf = 0; bf < buffer_size; bf++) buffer_out[bf] = 0.0;
}

// Takes a buffer as an argument, fills said buffer while rendering sound samples.
void fill_buffer(int buffer_size, float * buffer_out, float step) {
    for (int bf = 0; bf < buffer_size; bf++) {
        for (int id = 0; id < VOICES_CNT; id++) {
            if (tab[id] != NULL) {
                poly_voice_t * n = tab[id];
                // Update energy
                if (!n->hold && n->energy > 0) n->energy -= 1;
                // Update phase
                n->phase += n->freq * step;
                if (n->phase >= 2.0) n->phase = 0.0;
                // Generate a wave
                float last_sample = ( (float)n->energy / (float)ENERGY_MAX ) * phase_to_sin(n->phase);
                // Mix and store sample in a buffer
                buffer_out[bf] += last_sample / (float)VOICES_CNT;
            }
            if (tab[id] != NULL) {
                if (tab[id]->energy <= 0 && tab[id]->hold == 0) {
                    free_note(id);
                }
            }
        }
    }
}

// Free each polyphony voice memory
void free_voice_memory() {
    for (int id = 0; id < VOICES_CNT; id++) {
        if (tab[id] != NULL) free_note(id);
    }
}

// Checks if a polyphony voice is available for using.
// Returns a voice ID if it is available.
// Returns -1 otherwise.
int voice_available() {
    int result = -1;
    for (int id = 0; id < VOICES_CNT; id++) {
       if (tab[id] == NULL) {
           result = id;
           break;
       }
    }
    return result;
}

// Add a note to a voice array
int new_note(unsigned int note, float freq, int hold, unsigned int energy) {
    // Search for an unused voice
    int id = voice_available();
    if (id > -1) {
        // Allocate memory for a new voice
        poly_voice_t * n = malloc(sizeof(poly_voice_t));
        // Fill voice parameters
        n->midi_note = note;
        n->freq = freq;
        n->hold = hold;
        n->phase = 0.0;
        n->energy = energy;
        // Store a voice
        tab[id] = n;
        return id;
    }
    return id;
}

// Stops holding a note so envelope can update.
// Accepts a voice ID.
void drop_note_by_id(int id) {
    assert(tab[id] != NULL);
    tab[id]->hold = 0;
}

// Stops holding a note, so its envelope can update.
//
// Accepts a MIDI note.
void drop_note_by_value(unsigned int note) {
    for (int id = 0; id < VOICES_CNT; id++) {
        if (tab[id] != NULL && tab[id]->midi_note == note) {
            drop_note_by_id(id);
            break;
        }
    }
}

// Stops holding all instances of a polyphony note,
// so their envelopes can update.
//
// Accepts a MIDI note.
void drop_notes_by_value(unsigned int note) {
    for (int id = 0; id < VOICES_CNT; id++) {
        if (tab[id] != NULL && tab[id]->midi_note == note) {
            drop_note_by_id(id);
        }
    }
}

// Locates a note in a polyphony tab
int find_midi_note_in_tab(unsigned int note) {
    int result = -1;
    for (int id = 0; id < VOICES_CNT; id++) {
        if (tab[id] != NULL && tab[id]->midi_note == note) {
            result = id;
            break;
        }
    }
    return result;
}

// Locates a held note in a polyphony tab
int find_held_midi_note_in_tab(unsigned int note) {
    int result = -1;
    for (int id = 0; id < VOICES_CNT; id++) {
        if (tab[id] != NULL && tab[id]->hold == 1 && tab[id]->midi_note == note) {
            result = id;
            break;
        }
    }
    return result;
}


// Convert a float semitone value to a float frequency value
float semitone_to_freq(float s) {
    return pow(2, (s - 69) / 12.) * 440.0;
}

// Converts a MIDI note to a frequency value in Hz.
//
// Original by DFL:
// https://www.musicdsp.org/en/latest/Other/125-midi-note-frequency-conversion.html
double midi_note_to_freq(char keynum) {
    return 440.0 * pow(2.0, ((double)keynum - 69.0) / 12.0);
}

// A function to display active polyphony voices.
// Should display results immediately.
void scan_voices() {
    int target_voice = -1;
    for (int id = 0; id < VOICES_CNT; id++) {
       if (tab[id] == NULL) {
           printf("-");
           if (target_voice == -1) target_voice = id;
       } else {
           printf("+");
       }
    }
    printf(" %d \n", target_voice);
    fflush(stdout);
}

/*
 * Functions which are not used directly by the example
 */

// Set energy for a voice
void set_energy(int id, unsigned int energy) {
    if (tab[id] != NULL) tab[ id ]->energy = energy;
}

// Get a frequency by a voice id
float get_freq(int id) {
    if (tab[id] != NULL) return tab[ id ]->freq;
    else return -1;
}

// Get energy for a voice
unsigned int get_energy(int id) {
    if (tab[id] != NULL) return tab[ id ]->energy;
    else return -1;
}

// Get a maximum possible amount of voices
int get_max_notes() {
    return VOICES_CNT;
}

// Check if a voice by an integer id is available for a new note
int empty_id(int id) {
    return tab[id] == NULL;
}

// Convert a float frequency value to a float semitone value
float freq_to_semitone(float f) {
    return 12 * log2(f / 440.0) + 69;
}

// Returns a modulo 12 result for a float input
float mod_12(float f) {
    while (f >= 12) f -= 12;
    while (f < 0) f += 12;
    return f;
}
