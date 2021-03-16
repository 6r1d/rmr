/*
 * An polyphony example to play around.
 * Based on Acratone's polyphony and Libsoundio sine example.
 * https://github.com/Saegor/acratone/blob/master/sound.c
 * https://github.com/andrewrk/libsoundio/blob/master/example/sio_sine.c
 *
 * Run like:
 * ./main --latency 0.1
 * or even
 * ./main --latency 0.01
 */


#include <soundio/soundio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "sample.h"

#include "midi/midi_handling.h"

#include "poly_gen.h"

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
        printf(" %f ", midi_note_to_freq((char)buf[1]));
        printf(" v %02x ", buf[2]); // velocity
        if (buf[1] > -1 && buf[1] < 128 && find_held_midi_note_in_tab(buf[1]) == -1) new_note(buf[1], midi_note_to_freq((char)buf[1]), 1, ENERGY_MAX);
    }
    else if (buf[0] == MIDI_MSG_NOTE_OFF) {
        printf("Note Off");
        printf(" n %d ", buf[1]); // note
        printf(" z?%02x ", buf[2]); // zero
        if (buf[1] > -1 && buf[1] < 128) drop_notes_by_value(buf[1]);
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
        while (g_async_queue_length(input_data->midi_async_queue)) {
            msg = g_async_queue_try_pop(input_data->midi_async_queue);
            if (msg != NULL) {
                handle_midi_buffer(msg->buf, msg->count);
                free_midi_message(msg);
            }
        }
        while (g_async_queue_length(input_data->error_async_queue)) {
            err_msg = g_async_queue_try_pop(input_data->midi_async_queue);
            if (err_msg != NULL) free_error_message(err_msg);
        }
}

static int usage(char *exe) {
    fprintf(stderr, "Usage: %s [options]\n"
            "Options:\n"
            "  [--backend dummy|alsa|pulseaudio|jack|coreaudio|wasapi]\n"
            "  [--device id]\n"
            "  [--raw]\n"
            "  [--name stream_name]\n"
            "  [--latency seconds]\n"
            "  [--sample-rate hz]\n"
            , exe);
    return 1;
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



    // Clear the buffer
    clear_buffer(frame_count_max, buffer_out);

    // Calculate new values for the buffer
    for (int bf = 0; bf < frame_count_max; bf++) {
        if (frame_count_max < 5000 && bf % 10 == 0) scan_queue();
        for (int id = 0; id < VOICES_CNT; id++) {
            if (tab[id] != NULL) {
                poly_voice_t * n = tab[id];
                // Update energy
                if (!n->hold && n->energy > 0) n->energy -= 1;
                // Update phase
                n->phase += n->freq * step;
                // Reset phase to prevent (potential) overflow.
                if (n->phase >= 1.0) n->phase -= 1.0f;
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

    // Fill samples in a buffer
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

static void underflow_callback(struct SoundIoOutStream *outstream) {
    static int count = 0;
    fprintf(stderr, "underflow %d\n", count++);
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






    char *exe = argv[0];
    enum SoundIoBackend backend = SoundIoBackendNone;
    char *device_id = NULL;
    bool raw = false;
    char *stream_name = NULL;
    double latency = 0.0;
    int sample_rate = 0;
    for (int i = 1; i < argc; i += 1) {
        char *arg = argv[i];
        if (arg[0] == '-' && arg[1] == '-') {
            if (strcmp(arg, "--raw") == 0) {
                raw = true;
            } else {
                i += 1;
                if (i >= argc) {
                    return usage(exe);
                } else if (strcmp(arg, "--backend") == 0) {
                    if (strcmp(argv[i], "dummy") == 0) {
                        backend = SoundIoBackendDummy;
                    } else if (strcmp(argv[i], "alsa") == 0) {
                        backend = SoundIoBackendAlsa;
                    } else if (strcmp(argv[i], "pulseaudio") == 0) {
                        backend = SoundIoBackendPulseAudio;
                    } else if (strcmp(argv[i], "jack") == 0) {
                        backend = SoundIoBackendJack;
                    } else {
                        fprintf(stderr, "Invalid backend: %s\n", argv[i]);
                        return 1;
                    }
                } else if (strcmp(arg, "--device") == 0) {
                    device_id = argv[i];
                } else if (strcmp(arg, "--name") == 0) {
                    stream_name = argv[i];
                } else if (strcmp(arg, "--latency") == 0) {
                    latency = atof(argv[i]);
                } else if (strcmp(arg, "--sample-rate") == 0) {
                    sample_rate = atoi(argv[i]);
                } else {
                    return usage(exe);
                }
            }
        } else {
            return usage(exe);
        }
    }

    struct SoundIo *soundio = soundio_create();
    if (!soundio) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    int err = (backend == SoundIoBackendNone) ?
        soundio_connect(soundio) : soundio_connect_backend(soundio, backend);

    if (err) {
        fprintf(stderr, "Unable to connect to backend: %s\n", soundio_strerror(err));
        return 1;
    }

    fprintf(stderr, "Backend: %s\n", soundio_backend_name(soundio->current_backend));

    soundio_flush_events(soundio);

    int selected_device_index = -1;
    if (device_id) {
        int device_count = soundio_output_device_count(soundio);
        for (int i = 0; i < device_count; i += 1) {
            struct SoundIoDevice *device = soundio_get_output_device(soundio, i);
            bool select_this_one = strcmp(device->id, device_id) == 0 && device->is_raw == raw;
            soundio_device_unref(device);
            if (select_this_one) {
                selected_device_index = i;
                break;
            }
        }
    } else {
        selected_device_index = soundio_default_output_device_index(soundio);
    }

    if (selected_device_index < 0) {
        fprintf(stderr, "Output device not found\n");
        return 1;
    }

    struct SoundIoDevice *device = soundio_get_output_device(soundio, selected_device_index);
    if (!device) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    fprintf(stderr, "Output device: %s\n", device->name);

    if (device->probe_error) {
        fprintf(stderr, "Cannot probe device: %s\n", soundio_strerror(device->probe_error));
        return 1;
    }

    struct SoundIoOutStream *outstream = soundio_outstream_create(device);
    if (!outstream) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    outstream->write_callback = write_callback;
    outstream->underflow_callback = underflow_callback;
    outstream->name = stream_name;
    outstream->software_latency = latency;
    outstream->sample_rate = sample_rate;

    if (soundio_device_supports_format(device, SoundIoFormatFloat32NE)) {
        outstream->format = SoundIoFormatFloat32NE;
        write_sample = write_sample_float32ne;
    } else if (soundio_device_supports_format(device, SoundIoFormatFloat64NE)) {
        outstream->format = SoundIoFormatFloat64NE;
        write_sample = write_sample_float64ne;
    } else if (soundio_device_supports_format(device, SoundIoFormatS32NE)) {
        outstream->format = SoundIoFormatS32NE;
        write_sample = write_sample_s32ne;
    } else if (soundio_device_supports_format(device, SoundIoFormatS16NE)) {
        outstream->format = SoundIoFormatS16NE;
        write_sample = write_sample_s16ne;
    } else {
        fprintf(stderr, "No suitable device format available.\n");
        return 1;
    }

    if ((err = soundio_outstream_open(outstream))) {
        fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
        return 1;
    }

    fprintf(stderr, "Software latency: %f\n", outstream->software_latency);
    fprintf(stderr, "'q\\n' - quit\n");

    if (outstream->layout_error)
        fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

    if ((err = soundio_outstream_start(outstream))) {
        fprintf(stderr, "unable to start device: %s\n", soundio_strerror(err));
        return 1;
    }

    for (;;) {
        soundio_flush_events(soundio);
        int c = getc(stdin);
        if (c == 'q') {
            break;
        } else if (c == '\r' || c == '\n') {
            // ignore
        } else {
            fprintf(stderr, "Unrecognized command: %c\n", c);
        }
    }

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
