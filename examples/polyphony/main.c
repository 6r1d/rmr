#include <soundio/soundio.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <stdlib.h>

#include "midi/midi_handling.h"

#include "poly_gen.h"

unsigned int r = 0;

Alsa_MIDI_data * data;
MIDI_in_data * input_data;
MIDI_message * msg;
error_message * err_msg;

RMR_Port_config * port_config;

unsigned char MIDI_MSG_NOTE_OFF = 0x80;
unsigned char MIDI_MSG_NOTE_ON = 0x90;
unsigned char MIDI_MSG_CONT_CTR = 0xB0;

void handle_midi_buffer(unsigned char * buf, long count) {
    // unsigned int byte_idx;
    if (buf[0] == MIDI_MSG_NOTE_ON) {
        printf("Note On");
        printf(" n %d ", buf[1]); // note
        printf(" %f ", midi_note_to_freq(buf[1]));
        printf(" v %02x ", buf[2]); // velocity
        new_note(midi_note_to_freq(buf[1]), 0, ENERGY_MAX);
    }
    else if (buf[0] == MIDI_MSG_NOTE_OFF) {
        printf("Note Off");
        printf(" n %d ", buf[1]); // note
        printf(" z?%02x ", buf[2]); // zero
        if (buf[0] > -1 && buf[0] < 128) drop_note_by_value(buf[1]);
    }
    else if (buf[0] == MIDI_MSG_CONT_CTR) {
        printf("Continuous controller");
        printf(" a %d ", buf[1]); // addr
        printf(" v %d ", buf[2]); // value
    }
    printf(" ");
    printf("\n");
    fflush( stdout );
}

void scan_queue() {
        while (g_async_queue_length_unlocked(input_data->midi_async_queue)) {
            msg = g_async_queue_try_pop(input_data->midi_async_queue);
            if (msg != NULL) {
                handle_midi_buffer(msg->buf, msg->count);
                free_midi_message(msg);
            }
        }
        while (g_async_queue_length_unlocked(input_data->error_async_queue)) {
            err_msg = g_async_queue_try_pop(input_data->midi_async_queue);
            if (err_msg != NULL) free_error_message(err_msg);
        }
}

static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
    const struct SoundIoChannelLayout *layout = &outstream->layout;
    float sample_rate = outstream->sample_rate;
    struct SoundIoChannelArea *areas;
    int frames_left = frame_count_max;
    int err;

    // Sound buffer
    float * buffer_out = malloc(frame_count_max * sizeof(float));
    // Smallest period possible for that sample rate,
    // used to update a phase
    // probably equal to seconds_per_frame
    float step = 1.0f / (float) sample_rate;



    if (r < UINT_MAX) r++;
    else r = 0;

    // Clear the buffer
    clear_buffer(frame_count_max, buffer_out);
    for (int bf = 0; bf < frame_count_max; bf++) {
        if (frame_count_max < 5000 && bf % 10 == 0) scan_queue();
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

    while (frames_left > 0) {
        int frame_count = frames_left;

        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        if (!frame_count) break;

        for (int frame = 0; frame < frame_count; frame += 1) {
            float sample = buffer_out[frame] * 1.0f;
            for (int channel = 0; channel < layout->channel_count; channel += 1) {
                float *ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
                *ptr = sample;
            }
        }

        if ((err = soundio_outstream_end_write(outstream))) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        frames_left -= frame_count;
    }

    free(buffer_out);
}

int main(int argc, char **argv) {
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





    int err;

    struct SoundIo *soundio = soundio_create();

    if (!soundio) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    if ((err = soundio_connect(soundio))) {
        fprintf(stderr, "error connecting: %s\n", soundio_strerror(err));
        return 1;
    }

    soundio_flush_events(soundio);

    int default_out_device_index = soundio_default_output_device_index(soundio);
    if (default_out_device_index < 0) {
        fprintf(stderr, "no output device found\n");
        return 1;
    }

    struct SoundIoDevice *device = soundio_get_output_device(soundio, default_out_device_index);
    if (!device) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    fprintf(stderr, "Output device: %s\n", device->name);

    struct SoundIoOutStream *outstream = soundio_outstream_create(device);
    if (!outstream) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }
    outstream->format = SoundIoFormatFloat32NE;
    outstream->write_callback = write_callback;

    if ((err = soundio_outstream_open(outstream))) {
        fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
        return 1;
    }

    if (outstream->layout_error)
        fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

    if ((err = soundio_outstream_start(outstream))) {
        fprintf(stderr, "unable to start device: %s\n", soundio_strerror(err));
        return 1;
    }

    for (;;)
        soundio_wait_events(soundio);

    soundio_outstream_destroy(outstream);
    soundio_device_unref(device);
    soundio_destroy(soundio);

    free_voice_memory();






    // Close a MIDI input port,
    // shutdown the input thread,
    // do cleanup
    destroy_midi_input(data, input_data);

    // Destroy a port configuration
    destroy_port_config(port_config);

    return 0;
}
