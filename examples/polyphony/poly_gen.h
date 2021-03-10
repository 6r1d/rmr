/*
 * Polyphony part, based on Acratone synth, slightly changed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

// 16 voices
#define VOICES_CNT 16
// Sample rate constant for Alsa
#define SAMPLE_RATE 44100

#define ENERGY_MAX 20000

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

// Converts a float phase value to a float saw value
float phase_to_sin(float p) {
    return sinf(2 * M_PI * p);
}

// Remove a note from *ACTIVE voices?*
void free_note(int id) {
    // Throw an error if a voice does not exist
    assert(tab[id] != NULL);
    // Free the memory
    free(tab[id]);
    // Clean the voice
    tab[id] = NULL;
}

void clear_buffer(int buffer_size, float * buffer_out) {
    for (int bf = 0; bf < buffer_size; bf++) buffer_out[bf] = 0.0;
}

// Fills a buffer with samples
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

void free_voice_memory() {
    // Free voice memory before exiting
    for (int id = 0; id < VOICES_CNT; id++) {
        if (tab[id] != NULL) free_note(id);
    }
}

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
int new_note(float freq, int hold, unsigned int energy) {
    // Search for an unused voice
    int id = voice_available();
    if (id > -1) {
        // Allocate memory for a new voice
        poly_voice_t * n = malloc(sizeof(poly_voice_t));
        // Fill voice parameters
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

// Stop holding a voice
void drop_note(int id) {
    assert(tab[id] != NULL);
    tab[id]->hold = 0;
}

void drop_note_by_value(unsigned int note) {
    for (int id = 0; id < VOICES_CNT; id++) {
        if (tab[id]->midi_note == note) {
            free_note(id);
            break;
        }
    }
}

// Convert a float semitone value to a float frequency value
float semitone_to_freq(float s) {
    return pow(2, (s - 69) / 12.) * 440.0;
}

// Original: musicdsp.org
double midi_note_to_freq(char keynum) {
    return 440.0 * pow(2.0, ((double)keynum - 69.0) / 12.0);
}

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

// Float modulo 12
float mod_12(float f) {
    while (f >= 12) f -= 12;
    while (f < 0) f += 12;
    return f;
}
