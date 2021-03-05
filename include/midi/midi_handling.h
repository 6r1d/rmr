#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <glib.h>
#include "asoundlib.h"
// C-related helpers, unrelated to core structures
#include "helpers.h"
// MIDI-related structures
#include "midi_structures.h"
// Logging utilities
#include "logging.h"
// Error handling utilities
#include "error_handling.h"

/**
 * A main include file
 */

/** An amount of nanoseconds in a second */
#define NANOSECONDS_IN_SECOND 1000000000
/** A constant that defines the beat tempo in microseconds */
#define QUEUE_TEMPO 600000
/** A constant that defines the base resolution of the ticks (pulses per quarter note) */
#define QUEUE_STATUS_PPQ 240

/**
 * Allocates memory for a :c:type:`MIDI_port` instance
 *
 * :param current_midi_port: a double pointer used to allocate memory for a :c:type:`MIDI_port` instance
 *
 * :since: v0.1
 */
void init_midi_port(MIDI_port ** current_midi_port) {
    * current_midi_port = NULL;
    * current_midi_port = malloc(sizeof(MIDI_port));
}

/**
 * Prints a name of :c:type:`snd_seq_client_info_t` and :c:type:`snd_seq_port_info_t` containers
 *
 * :param current_midi_port: a :c:type:`MIDI_port` instance to display
 *
 * :since: v0.1
 */
void print_midi_port(MIDI_port * current_midi_port) {
    printf(
        "%s %s\n",
        current_midi_port->client_info_name,
        current_midi_port->port_info_name
    );
}

/**
 * Deallocates memory for :c:type:`MIDI_message` instance
 *
 * :param msg: a :c:type:`MIDI_message` instance
 *
 * :since: v0.1
 */
void free_midi_message(MIDI_message * msg) {
    if (msg->buf)   g_free(msg->buf);
    g_free(msg);
}

/**
 * Allocates memory for an instance of :c:type:`Alsa_MIDI_data`
 * or throws an error
 *
 * :param amidi_data: a double pointer used to allocate memory for a :c:type:`Alsa_MIDI_data` instance
 *
 * :since: v0.1
 */
void init_amidi_data_instance(Alsa_MIDI_data ** amidi_data) {
    * amidi_data = NULL;
    * amidi_data = malloc(sizeof(Alsa_MIDI_data));
    if (amidi_data == NULL) slog("Start", "Unable to allocate memory for Alsa_MIDI_data instance.");
}

/**
 * Creates an assigns a MIDI message queue for a :c:type:`MIDI_in_data` instance
 *
 * :param input_data: a pointer containing a :c:type:`MIDI_in_data` instance for creating a :c:type:`GAsyncQueue` instance.
 *
 * :since: v0.1
 */
void assign_midi_queue(MIDI_in_data * input_data) {
    input_data->midi_async_queue = g_async_queue_new();
    g_async_queue_ref(input_data->midi_async_queue);
}

/**
 * Assign :c:type:`Alsa_MIDI_data` instance to :c:type:`MIDI_in_data` instance
 *
 * :param input_data: :c:type:`MIDI_in_data` instance
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 *
 * :since: v0.1
 */
void assign_midi_data(MIDI_in_data * input_data, Alsa_MIDI_data * amidi_data) {
    input_data->amidi_data = amidi_data;
}

/**
 * Sets a callback for MIDI input events.
 *
 * :param input_data: :c:type:`MIDI_in_data` instance
 * :param callback: :c:type:`MIDI_callback` instance
 * :param user_data: an optional pointer to additional data that is passed
 *                   to the callback function whenever it is called.
 *
 * :since: v0.1
 */
void set_MIDI_in_callback(
    MIDI_in_data * input_data,
    MIDI_callback callback,
    void *user_data
  ) {
    // Exit if a callback is already set.
    // Allows to detect code issues.
    if ( input_data->using_callback ) {
        slog("MIDI in", "callback function is already set.");
        exit(1);
        return;
    }
    // Exit if callback is NULL.
    // Allows to detect code issues.
    if ( !callback ) {
        slog("MIDI in", "callback function value is invalid.");
        exit(1);
        return;
    }
    // Configure input data
    input_data->user_callback = callback;
    input_data->user_data = user_data;
    input_data->using_callback = true;
}

/**
 * Add an :c:type:`error_message` instance to :c:type:`MIDI_in_data` instance
 *
 * :param input_data: a :c:type:`MIDI_in_data` instance containing a queue to send an error to
 * :param etype: error type string, for example, "V0001", "S0001"
 * :param msg: error message
 *
 * :since: v0.1
 */
void enqueue_error(MIDI_in_data * input_data, char * etype, char * msg) {
    error_message *err = g_new(error_message, 1);
    strncpy(err->error_type, etype, ERROR_MSG_ETYPE_SIZE);
    strncpy(err->message, msg, ERROR_MSG_TEXT_SIZE);
    g_async_queue_push(input_data->error_async_queue, err);
}

/**
 * Create an error queue, add to :c:type:`MIDI_in_data` instance
 *
 * :param input_data: a :c:type:`MIDI_in_data` instance to add an error queue to
 *
 * :since: v0.1
 */
void assign_error_queue(MIDI_in_data * input_data) {
    input_data->error_async_queue = g_async_queue_new();
    g_async_queue_ref(input_data->error_async_queue);
}

/**
 * Fill :c:type:`Alsa_MIDI_data` struct instance
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance to initialize
 * :param port_type: a port type to use, supports all values for :c:type:`mp_type_t`
 *
 * :returns: **0** on success, **-1** when :c:data:`port_type` is not
 *   :c:member:`mp_type_t.MP_IN`, :c:member:`mp_type_t.MP_VIRTUAL_IN`,
 *   :c:member:`mp_type_t.MP_OUT`, :c:member:`mp_type_t.MP_VIRTUAL_OUT`.
 *
 * :since: v0.1
 */
int init_amidi_data(Alsa_MIDI_data * amidi_data, mp_type_t port_type) {
    int result = 0;
    amidi_data->port_num = -1;
    amidi_data->vport = -1;
    if (port_type == MP_IN || port_type == MP_VIRTUAL_IN) {
        amidi_data->port_num = -1;
        amidi_data->vport = -1;
        amidi_data->subscription = 0;
        amidi_data->dummy_thread_id = pthread_self();
        amidi_data->thread = amidi_data->dummy_thread_id;
        amidi_data->trigger_fds[0] = -1;
        amidi_data->trigger_fds[1] = -1;
    } else if (port_type == MP_OUT || port_type == MP_VIRTUAL_OUT) {
        amidi_data->buffer_size = 32;
        amidi_data->coder = 0;
        if (port_type == MP_VIRTUAL_OUT) amidi_data->buffer = (unsigned char *)malloc(amidi_data->buffer_size);
        else amidi_data->buffer = 0;
    } else {
        slog("Start", "unknown port type.");
        result = -1;
    }
    return result;
}

// TODO this is done for ordering only, improve!
void set_client_name(Alsa_MIDI_data *amidi_data, const char *client_name);

/**
 * Creates a seq instance, assigns it to :c:type:`Alsa_MIDI_data`
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param client_name: client name string
 * :param port_type: a port type for a sequencer, supports all values for :c:type:`mp_type_t`
 *
 * :returns: **0** on success, **-1** on error.
 *
 * :since: v0.1
 */
int init_seq(
    Alsa_MIDI_data * amidi_data,
    const char * client_name,
    mp_type_t port_type
) {
    int result = 0;
    int seq_open_result;
    int seq_mode;
    snd_seq_t * seq;
    if      (port_type == MP_IN  || port_type == MP_VIRTUAL_IN)  seq_mode = SND_SEQ_OPEN_INPUT;
    else if (port_type == MP_OUT || port_type == MP_VIRTUAL_OUT) seq_mode = SND_SEQ_OPEN_OUTPUT;
    else    result = -1;
    do {
        if (result == -1) {
            slog("ALSA MIDI", "incorrect port type.");
            result = -1;
            break;
        }
        // Open the ALSA sequencer using snd_seq_t pointer (&seq),
        // in non-blocking mode, input or output mode depends on seq_mode variable
        seq_open_result = snd_seq_open(&seq, "default", seq_mode, SND_SEQ_NONBLOCK);
        // Check for an error
        if (seq_open_result < 0) {
            slog("ALSA MIDI", "error creating ALSA sequencer client object.");
            result = -1;
            break;
        }
        // Add seq to :c:type:`Alsa_MIDI_data` instance
        amidi_data->seq = seq;
        // Set a client name for Alsa seq interface
        set_client_name(amidi_data, client_name);
    }
    while (0);
    return result;
}

/**
 * Creates a named input queue, sets its tempo and other parameters.
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param queue_name: a name for a new named queue
 * :param port_config: an instance of port configuration: :c:type:`RMR_Port_config`
 *
 * :returns: **0** or **-1** when an error happens
 *
 * :since: v0.1
 */
int start_input_seq(
    Alsa_MIDI_data* amidi_data,
    const char* queue_name,
    RMR_Port_config * port_config
) {
    int result = 0;
    do {
        // Check if a pipe can be created, set trigger_fds.
        // Can break if there are too many open files in OS
        // or if too many file descriptors are used by a current process.
        if (pipe(amidi_data->trigger_fds) == -1) {
            slog("MIDI in", "error creating pipe objects.");
            result = -1;
            break;
        }
        // Create the input queue
        #ifndef AVOID_TIMESTAMPING
        amidi_data->queue_id = snd_seq_alloc_named_queue(amidi_data->seq, queue_name);
        // Set arbitrary tempo (mm=100) and resolution (240)
        snd_seq_queue_tempo_t * qtempo;
        snd_seq_queue_tempo_alloca(&qtempo);
        // Set a MIDI tempo for a queue
        snd_seq_queue_tempo_set_tempo(qtempo, port_config->queue_tempo);
        // Set the amount of pulses per quarter note
        snd_seq_queue_tempo_set_ppq(qtempo, port_config->queue_ppq);
        snd_seq_set_queue_tempo(amidi_data->seq, amidi_data->queue_id, qtempo);
        snd_seq_drain_output(amidi_data->seq);
        #endif
    }
    while (0);
    return result;
}

/**
 * Creates a MIDI event parser and a MIDI port.
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param port_name: a name to set for a new virtual output port
 *
 * :returns: **0** on success, **-1** on an error
 *
 * :since: v0.1
 */
int start_virtual_output_seq(Alsa_MIDI_data * amidi_data, const char * port_name) {
    int result = 0;
    do {
        // Create a MIDI event parser with a pre-set buffer size
        if (snd_midi_event_new(amidi_data->buffer_size, &amidi_data->coder) < 0) {
            result = -1;
            free(amidi_data);
            slog("MIDI out", "error initializing MIDI event parser.");
            break;
        }
        // Reset encoder and decoder of the MIDI event parser
        snd_midi_event_init(amidi_data->coder);
        // Open a virtual port
        amidi_data->vport = snd_seq_create_simple_port(
            amidi_data->seq, port_name,
            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION
        );
        // Throw an error
        if (amidi_data->vport < 0) {
            result = -1;
            free(amidi_data);
            slog("MIDI out", "error creating virtual port.");
            break;
        }
    }
    while(0);
    return result;
}

/**
 * Create a MIDI event parser for MIDI output, reset it.
 * Used by **start_output_port**.
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 *
 * :returns: **0** on success, **-1** on an error
 *
 * :since: v0.1
 */
int start_output_seq(Alsa_MIDI_data * amidi_data) {
    int result = 0;
    do {
        // Create a MIDI event parser with a pre-set buffer size
        if (snd_midi_event_new(amidi_data->buffer_size, &amidi_data->coder) < 0) {
            result = -1;
            free(amidi_data);
            slog("MIDI out", "error initializing MIDI event parser.");
            break;
        }
        // Allocate buffer memory using buffer_size
        amidi_data->buffer = (unsigned char *) malloc( amidi_data->buffer_size );
        if (amidi_data->buffer == NULL) {
            result = -1;
            free(amidi_data);
            slog("MIDI out", "error allocating buffer memory.");
            break;
        }
        // Reset MIDI encode/decode parsers
        snd_midi_event_init(amidi_data->coder);
    }
    while(0);
    return result;
}

/**
 * This function is used to count
 * or get the pinfo structure for a given port number.
 *
 * :param snd_seq_t: Alsa's :c:type:`snd_seq_t` instance
 * :param pinfo: Alsa's :c:type:`snd_seq_port_info_t` instance
 * :param type: Alsa MIDI port capabilities :c:data:`SND_SEQ_PORT_CAP_READ` | :c:data:`SND_SEQ_PORT_CAP_SUBS_READ` or
 *              :c:data:`SND_SEQ_PORT_CAP_WRITE` | :c:data:`SND_SEQ_PORT_CAP_SUBS_WRITE`
 * :param port_number: use -1 for counting ports and a port number to get port info (in that case a function returns 1).
 *
 * :returns: port count (or the amount of ports) when a negative port_number value is provided,
 *           1 when a port ID is provided and the port is found,
 *           0 when a port ID is not found
 */
unsigned int port_info(
    snd_seq_t * seq,
    snd_seq_port_info_t * pinfo,
    unsigned int type,
    int port_number
  ) {
    snd_seq_client_info_t *cinfo;
    int client;
    int count = 0;
    // Allocate a snd_seq_client_info_t container on stack
    snd_seq_client_info_alloca(&cinfo);
    // Set the client id of a client_info container
    snd_seq_client_info_set_client(cinfo, -1);
    // Query the next client
    while (snd_seq_query_next_client(seq, cinfo) >= 0) {
        client = snd_seq_client_info_get_client(cinfo);
        if (client == 0) continue;
        // Reset query info
        snd_seq_port_info_set_client(pinfo, client);
        snd_seq_port_info_set_port( pinfo, -1 );
        while (snd_seq_query_next_port( seq, pinfo ) >= 0) {
            unsigned int atyp = snd_seq_port_info_get_type(pinfo);
            if ( ( ( atyp & SND_SEQ_PORT_TYPE_MIDI_GENERIC ) == 0 ) &&
                 ( ( atyp & SND_SEQ_PORT_TYPE_SYNTH ) == 0 ) &&
                 ( ( atyp & SND_SEQ_PORT_TYPE_APPLICATION ) == 0 ) ) continue;
            unsigned int caps = snd_seq_port_info_get_capability(pinfo);
            if (( caps & type ) != type) continue;
            if (count == port_number) return 1;
            ++count;
        }
    }
    // If a negative port number was used, return the port count.
    if (port_number < 0) return count;
    // No port was found
    return 0;
}

/**
 * Counts midi ports for input and output types
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param type: Alsa MIDI port capabilities, like ":c:data:`SND_SEQ_PORT_CAP_READ` | :c:data:`SND_SEQ_PORT_CAP_SUBS_READ`"
 *              or ":c:data:`SND_SEQ_PORT_CAP_WRITE` | :c:data:`SND_SEQ_PORT_CAP_SUBS_WRITE`"
 *
 * :returns: MIDI port count
 *
 * :since: v0.1
 */
unsigned int get_midi_port_count(Alsa_MIDI_data * amidi_data, unsigned int type) {
    snd_seq_port_info_t * pinfo;
    // Allocate a snd_seq_port_info_t container on * pinfo
    // port information container
    snd_seq_port_info_alloca(&pinfo);
    // Count the ports
    return port_info(amidi_data->seq, pinfo, type, -1);
}

/**
 * Updates a **port** pointer to :c:type:`MIDI_port` instance from a selected `port_number`.
 *
 * :param port: a pointer to update
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param port_number: a port number
 * :param type: Alsa MIDI port capabilities :c:data:`SND_SEQ_PORT_CAP_READ` | :c:data:`SND_SEQ_PORT_CAP_SUBS_READ` or
 *              :c:data:`SND_SEQ_PORT_CAP_WRITE` | :c:data:`SND_SEQ_PORT_CAP_SUBS_WRITE`
 *
 * :since: v0.1
 */
void get_port_descriptor_by_id(
  MIDI_port * port,
  Alsa_MIDI_data * amidi_data,
  unsigned int port_number,
  unsigned int type
) {
    const char * client_info_name;
    const char * port_info_name;
    client_info_name = NULL;
    port_info_name = NULL;
    // Client information container
    snd_seq_client_info_t *cinfo;
    // Port information container
    snd_seq_port_info_t *pinfo;
    // Memory allocation
    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_alloca(&pinfo);
    // Update MIDI_port contents
    port->id = port_number;
    if (port_info(amidi_data->seq, pinfo, type, (int) port_number)) {
        int cnum = snd_seq_port_info_get_client(pinfo);
        snd_seq_get_any_client_info(amidi_data->seq, cnum, cinfo);
        client_info_name = snd_seq_client_info_get_name(cinfo);
        port_info_name = snd_seq_port_info_get_name(pinfo);
        strncpy(port->client_info_name, client_info_name, MAX_PORT_NAME_LEN - 1);
        strncpy(port->port_info_name, port_info_name, MAX_PORT_NAME_LEN - 1);
        port->port_info_client_id = snd_seq_port_info_get_client(pinfo);
        port->port_info_id = snd_seq_port_info_get_port(pinfo);
    } else {
        slog("MIDI port retrieval", "unable to retrieve port information. Exiting.");
        exit(1);
    }
}

/**
 * Free an Alsa MIDI event parser, reset its value in :c:type:`MIDI_in_data` instance,
 * set current :c:type:`MIDI_in_data` thread to **dummy_thread_id**
 *
 * :param input_data: a :c:type:`MIDI_in_data` instance
 *
 * :since: v0.1
 */
void deallocate_input_thread(struct MIDI_in_data * input_data) {
    snd_midi_event_free(input_data->amidi_data->coder);
    input_data->amidi_data->coder = 0;
    input_data->amidi_data->thread = input_data->amidi_data->dummy_thread_id;
}

/**
 * A start routine for :c:type:`alsa_MIDI_handler`.
 *
 * :param ptr: a void-pointer to :c:member:`MIDI_in_data`.
 *
 * :since: v0.1
 */
static void * alsa_MIDI_handler( void * ptr ) {
    struct MIDI_in_data * in_data = ptr;
    long byte_count;
    double time;
    bool continue_sysex = false;
    bool do_decode = false;
    int poll_fd_count;
    struct pollfd * poll_fds;
    // "bytes" pointer and timestamp value
    // are used to send the message to a queue.
    // They are reset at each received MIDI message.
    GArray * bytes;
    double timestamp = 0.0;
    bytes = g_array_sized_new(FALSE, FALSE, sizeof(unsigned char), 0);
    // Prepare a sequencer event record to convert a MIDI event to bytes
    snd_seq_event_t * ev;
    in_data->amidi_data->buffer_size = 32;
    // Create a MIDI event parser, process init error
    int result = snd_midi_event_new(0, &in_data->amidi_data->coder);
    if (result < 0) {
        in_data->do_input = false;
        enqueue_error(in_data, "S0001", "Error initializing MIDI event parser");
        return 0;
    }
    unsigned char *buffer = (unsigned char *)malloc(in_data->amidi_data->buffer_size);
    if ( buffer == NULL ) {
        in_data->do_input = false;
        snd_midi_event_free(in_data->amidi_data->coder);
        in_data->amidi_data->coder = 0;
        slog("Alsa MIDI handler", "Error initializing buffer memory.");
        return 0;
    }
    // Reset MIDI encode / decode parsers
    snd_midi_event_init(in_data->amidi_data->coder);
    // Suppress running status messages
    snd_midi_event_no_status(in_data->amidi_data->coder, 1);
    // Get the number of poll descriptors
    poll_fd_count = 1 + snd_seq_poll_descriptors_count(
      in_data->amidi_data->seq, POLLIN
    );
    poll_fds = (struct pollfd*)alloca(poll_fd_count * sizeof( struct pollfd ));
    snd_seq_poll_descriptors(in_data->amidi_data->seq, poll_fds + 1, poll_fd_count - 1, POLLIN);
    poll_fds[0].fd = in_data->amidi_data->trigger_fds[0];
    poll_fds[0].events = POLLIN;
    // Handle MIDI input while no errors or
    // interruptions are present
    while ( in_data->do_input ) {
        // Poll MIDI file descriptor
        if ( snd_seq_event_input_pending( in_data->amidi_data->seq, 1 ) == 0 ) {
            // No data pending
            if ( poll( poll_fds, poll_fd_count, -1) >= 0 ) {
                if ( poll_fds[0].revents & POLLIN ) {
                    bool dummy;
                    int res = read( poll_fds[0].fd, &dummy, sizeof(dummy) );
                    (void) res;
                }
            }
            continue;
        }
        // If here, there should be data.
        result = snd_seq_event_input( in_data->amidi_data->seq, &ev );
        if ( result == -ENOSPC ) {
            slog("Alsa MIDI handler", "input buffer overrun.");
            continue;
        } else if ( result <= 0 ) {
            slog("Alsa MIDI handler", "unknown MIDI input error.");
            continue;
        }
        // Creates new array for input if no sysex message is continued
        if ( !continue_sysex ) {
            bytes = g_array_sized_new(FALSE, FALSE, sizeof(unsigned char), 0);
        }
        // Determine if event bytes should be decoded
        do_decode = false;
        switch ( ev->type ) {
        case SND_SEQ_EVENT_PORT_SUBSCRIBED:
            break;
        case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
            break;
        // MIDI time code
        case SND_SEQ_EVENT_QFRAME:
            if ( !( in_data->ignore_flags & 0x02 ) ) do_decode = true;
            break;
        // 0xF9 ... MIDI timing tick
        case SND_SEQ_EVENT_TICK:
            if ( !( in_data->ignore_flags & 0x02 ) ) do_decode = true;
            break;
        // 0xF8 ... MIDI timing (clock) tick
        case SND_SEQ_EVENT_CLOCK:
            if ( !( in_data->ignore_flags & 0x02 ) ) do_decode = true;
            break;
        // Active sensing
        case SND_SEQ_EVENT_SENSING:
            if ( !( in_data->ignore_flags & 0x04 ) ) do_decode = true;
            break;
        case SND_SEQ_EVENT_SYSEX:
            if (in_data->ignore_flags & 0x01) break;
            if (ev->data.ext.len > in_data->amidi_data->buffer_size) {
                in_data->amidi_data->buffer_size = ev->data.ext.len;
                free( buffer );
                buffer = (unsigned char *) malloc(
                  in_data->amidi_data->buffer_size
                );
                if ( buffer == NULL ) {
                    in_data->do_input = false;
                    slog("Alsa MIDI handler", "error resizing buffer memory.");
                    break;
                }
            }
            do_decode = true;
            break;

        default:
            do_decode = true;
        }

        if ( do_decode ) {
            // Decode Alsa MIDI event to a byte buffer
            byte_count = snd_midi_event_decode(in_data->amidi_data->coder, buffer, in_data->amidi_data->buffer_size, ev);
            // Add a timestamp
            if ( byte_count > 0 ) {
                size_t sysex_array_pos;
                if ( !continue_sysex ) sysex_array_pos = 0;
                else sysex_array_pos = bytes->len;
                for(; sysex_array_pos < byte_count; sysex_array_pos++) {
                    g_array_append_val(bytes, buffer[sysex_array_pos]);
                }
                unsigned char last_byte = get_last_bytearray_byte(bytes);
                continue_sysex = ( (ev->type == SND_SEQ_EVENT_SYSEX) && (last_byte != 0xF7) );
                // Calculate a timestamp using ALSA sequencer event time data
                if ( !continue_sysex ) {
                    timestamp = 0.0;
                    snd_seq_real_time_t x = ev->time.time;
                    // Temp var y is timespec because computation requires signed types,
                    // while snd_seq_real_time_t has unsigned types.
                    struct timespec y;
                    // Perform the carry for the later subtraction by updating y.
                    y.tv_nsec = in_data->amidi_data->last_time.tv_nsec;
                    y.tv_sec = in_data->amidi_data->last_time.tv_sec;
                    if ( x.tv_nsec < y.tv_nsec ) {
                        int nsec = (y.tv_nsec - (int)x.tv_nsec) / NANOSECONDS_IN_SECOND + 1;
                        y.tv_nsec -= NANOSECONDS_IN_SECOND * nsec;
                        y.tv_sec += nsec;
                    }
                    if (x.tv_nsec - y.tv_nsec > NANOSECONDS_IN_SECOND) {
                        int nsec = ((int)x.tv_nsec - y.tv_nsec) / NANOSECONDS_IN_SECOND;
                        y.tv_nsec += NANOSECONDS_IN_SECOND * nsec;
                        y.tv_sec -= nsec;
                    }
                    // Compute the time difference
                    time = (int)x.tv_sec - y.tv_sec + ((int)x.tv_nsec - y.tv_nsec) * 1e-9;
                    in_data->amidi_data->last_time = ev->time.time;
                    if (in_data->first_message == true) in_data->first_message = false;
                    else timestamp = time;
                } else {
                    enqueue_error(
                        in_data,
                        "V0001",
                        "Event parsing error or not a MIDI event"
                    );
                }
            }
        }
        if (bytes->len == 0 || continue_sysex) continue;
        // Convert current message buffer
        unsigned char * buf = NULL;
        long count = bytes->len;
        if (count > 0) {
            // Allocate memory for an unsigned char pointer buffer.
            buf = calloc(bytes->len, sizeof(unsigned char));
            // Copy bytes
            for (unsigned int msg_byte_idx = 0; msg_byte_idx < bytes->len; msg_byte_idx++) {
                buf[msg_byte_idx] = g_array_index(bytes, unsigned char, msg_byte_idx);
            }
        }
        // Free GArray memory
        g_array_free(bytes, true);
        // Send data to a callback or a queue
        if (in_data->using_callback) {
            MIDI_callback callback = (MIDI_callback) in_data->user_callback;
            callback(timestamp, buf, count, in_data->user_data);
        } else {
            MIDI_message * message = g_new(MIDI_message, 1);
            message->buf = buf;
            message->count = count;
            message->timestamp = timestamp;
            g_async_queue_push(in_data->midi_async_queue, message);
        }
    }
    // Free the memory, allocated for a buffer
    if (buffer) free(buffer);
    // Destroy thread data
    deallocate_input_thread(in_data);
    // Stop thread with no error
    return 0;
}

/**
 * Opens a MIDI port with a given name, creates a thread and queues for it.
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param port_name: a char array pointer pointing to a port name string
 * :param input_data: a :c:type:`MIDI_in_data` instance
 *
 * :returns: 0 on success
 *
 * :since: v0.1
 */
int open_virtual_port(Alsa_MIDI_data * amidi_data, const char *port_name, MIDI_in_data * input_data) {
    int result = 0;
    do {
        // Configure seq inteface to create a virtual port
        if (amidi_data->vport < 0) {
            snd_seq_port_info_t *pinfo;
            snd_seq_port_info_alloca(&pinfo);
            snd_seq_port_info_set_capability(
                pinfo,
                SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE
            );
            snd_seq_port_info_set_type(
                pinfo,
                SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION
            );
            snd_seq_port_info_set_midi_channels(pinfo, 16);
            #ifndef AVOID_TIMESTAMPING
            snd_seq_port_info_set_timestamping(pinfo, 1);
            snd_seq_port_info_set_timestamp_real(pinfo, 1);
            snd_seq_port_info_set_timestamp_queue(pinfo, amidi_data->queue_id);
            #endif
            snd_seq_port_info_set_name(pinfo, port_name);
            amidi_data->vport = snd_seq_create_port(amidi_data->seq, pinfo);
            if (amidi_data->vport < 0) {
                slog("ALSA MIDI in", "error creating virtual port, unable to initialize sequencer.");
                result = -1;
                break;
            }
            amidi_data->vport = snd_seq_port_info_get_port(pinfo);
        }
        // Open a thread to handle a virtual port input
        if (input_data->do_input == false) {
            // Wait for old thread to stop, if still running
            if ( !pthread_equal(amidi_data->thread, amidi_data->dummy_thread_id) )
                pthread_join(amidi_data->thread, NULL);
            // Start the input queue
            #ifndef AVOID_TIMESTAMPING
            snd_seq_start_queue(amidi_data->seq, amidi_data->queue_id, NULL);
            snd_seq_drain_output(amidi_data->seq);
            #endif
            // Start our MIDI input thread.
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
            // Mark data input thread is active
            input_data->do_input = true;
            int err = pthread_create(&amidi_data->thread, &attr, alsa_MIDI_handler, input_data);
            pthread_attr_destroy(&attr);
            // Exit if thread init failed
            if ( err ) {
                if ( amidi_data->subscription ) {
                    snd_seq_unsubscribe_port( amidi_data->seq, amidi_data->subscription );
                    snd_seq_port_subscribe_free( amidi_data->subscription );
                    amidi_data->subscription = 0;
                }
                input_data->do_input = false;
                slog("Alsa MIDI input", "error starting MIDI input thread while creating a virtual port.");
                result = -1;
                break;
            }
        }
    } while(0);
    return result;
}

/**
 * Sends a MIDI message using a provided :c:type:`Alsa_MIDI_data` instance
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param message: a MIDI message to be sent
 * :param size: a size of MIDI message in bytes
 *
 * :returns: **0** on success, **-1** on an error
 *
 * :since: v0.1
 */
int send_midi_message(Alsa_MIDI_data * amidi_data, const unsigned char * message, size_t size) {
    int result = 0;
    unsigned int byte_count = (unsigned int)size;
    do {
        if (byte_count > amidi_data->buffer_size) {
            amidi_data->buffer_size = byte_count;
            int buf_resize_result = snd_midi_event_resize_buffer(amidi_data->coder, byte_count);
            if (buf_resize_result != 0) {
                slog("send_midi_message", "ALSA error resizing MIDI event buffer.");
                result = -1;
                break;
            }
            free(amidi_data->buffer);
            amidi_data->buffer = (unsigned char *) malloc(amidi_data->buffer_size);
            if (amidi_data->buffer == NULL) {
                slog("send_midi_message", "error allocating buffer memory.");
                result = -1;
                break;
            }
        }
        for (unsigned int i=0; i<byte_count; ++i) {
            amidi_data->buffer[i] = (unsigned char)message[i];
        }
        unsigned int offset = 0;
        while (offset < byte_count) {
            // A sequencer event record for a message
            snd_seq_event_t ev;
            snd_seq_ev_clear(&ev);
            snd_seq_ev_set_source(&ev, amidi_data->vport);
            snd_seq_ev_set_subs(&ev);
            snd_seq_ev_set_direct(&ev);
            long event_decoding_result = snd_midi_event_encode(
                amidi_data->coder,
                amidi_data->buffer + offset,
                (long)(byte_count - offset),
                &ev
            );
            if (event_decoding_result < 0) {
                slog("send_midi_message", "event parsing error.");
                result = -1;
                break;
            }
            if (ev.type == SND_SEQ_EVENT_NONE) {
                slog("send_midi_message", "incomplete message.");
                result = -1;
                break;
            }
            offset += event_decoding_result;
            // Send the event.
            int event_output_result = snd_seq_event_output( amidi_data->seq, &ev );
            if (event_output_result < 0) {
                slog("send_midi_message", "error sending MIDI message to port.");
                result = -1;
                break;
            }
        }
        snd_seq_drain_output(amidi_data->seq);
    } while(0);
    return result;
}

/**
 * Finds a MIDI port (port) by a given substring.
 * Matches a substring in :c:member:`MIDI_port.client_info_name` attribute.
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param port: :c:type:`MIDI_port` instance
 * :param substr: a port substring
 * :param write: do we look for a port we write into or read from?
 *
 * :returns: **1** when a port was found, **-1** on an error
 *
 * :since: v0.1
 */
int find_midi_port(
  Alsa_MIDI_data * amidi_data,
  MIDI_port * port,
  const char * substr,
  bool write
) {
    int result = -1;
    unsigned int port_count;
    if (!write) {
        port_count = get_midi_port_count(
            amidi_data,
            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ
        );
    } else {
        port_count = get_midi_port_count(
            amidi_data,
            SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE
        );
    }
    unsigned int port_idx;
    MIDI_port * current_midi_port = 0;
    for (port_idx = 0; port_idx < port_count; port_idx++) {
        current_midi_port = malloc(sizeof (MIDI_port));
        if (!write) {
            get_port_descriptor_by_id(
                current_midi_port,
                amidi_data,
                port_idx,
                SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ
            );
        } else {
            get_port_descriptor_by_id(
                current_midi_port,
                amidi_data,
                port_idx,
                SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE
            );
        }
        if (strstr(current_midi_port->client_info_name, substr) != NULL) {
            port->id = current_midi_port->id;
            strncpy(
              port->client_info_name,
              current_midi_port->client_info_name,
              MAX_PORT_NAME_LEN - 1
            );
            strncpy(
              port->port_info_name,
              current_midi_port->port_info_name,
              MAX_PORT_NAME_LEN - 1
            );
            port->port_info_client_id = current_midi_port->port_info_client_id;
            port->port_info_id = current_midi_port->port_info_id;
            result = 1;
            free(current_midi_port);
            break;
        } else {
            // Free the memory IF we don't need MIDI_port instance later
            free(current_midi_port);
        }
    }
    return result;
}

/**
 * Opens a MIDI port by its number.
 *
 * :param in: should the function look for input or output ports?
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param port_number: number of a port to look for
 * :param port_name: a name of a port to set using :c:func:`snd_seq_port_info_set_name`
 * :param input_data: a :c:type:`MIDI_in_data` instance
 *
 * :returns: **0** on success, **-1** on an error
 *
 * :since: v0.1
 */
int open_port(bool in, Alsa_MIDI_data * amidi_data, unsigned int port_number, const char *port_name, MIDI_in_data * input_data) {
    snd_seq_port_info_t * pinfo;
    snd_seq_addr_t sender, receiver;
    unsigned int midi_port_count;
    int32_t port_mode;
    int result = 0;
    do {
        // Null pointer check
        if (in && !input_data) {
            slog("Alsa MIDI in", "got a NULL pointer for input_data argument in open_port function.");
            result = -1;
        }
        // Throw an error if a port is already open
        if (amidi_data->port_connected) {
            if (in) slog("Alsa MIDI in", "opening an input port: a valid connection already exists.");
            else slog("Alsa MIDI out", "opening an output port: a valid connection already exists.");
            result = -1;
        }
        // Set port mode (read or write) in seq's format
        if (in) port_mode = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
        else    port_mode = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;
        // Count MIDI ports for a selected mode
        midi_port_count = get_midi_port_count(amidi_data, port_mode);
        // Throw an error if no ports are found
        if (midi_port_count < 1) {
            if (in) slog("Alsa MIDI in",  "no MIDI input sources found while opening a port.");
            else    slog("Alsa MIDI out", "no MIDI output sources found while opening a port.");
            result = -1;
        }
        // Allocate a snd_seq_port_info_t port information container on stack
        snd_seq_port_info_alloca(&pinfo);
        // Load port information
        if (port_info(amidi_data->seq, pinfo, port_mode, (int) port_number) == 0) {
            // Display an error if port number is not available
            if (in) slog("Alsa MIDI in", "invalid port number for port opening.");
            else    slog("Alsa MIDI out", "invalid port number for port opening.");
            result = -1;
        }
        // Configure sender and receiver
        if (in) {
            sender.client = snd_seq_port_info_get_client(pinfo);
            sender.port = snd_seq_port_info_get_port(pinfo);
            receiver.client = snd_seq_client_id(amidi_data->seq);
        } else {
            receiver.client = snd_seq_port_info_get_client(pinfo);
            receiver.port = snd_seq_port_info_get_port(pinfo);
            sender.client = snd_seq_client_id(amidi_data->seq);
        }
        // Allocate a snd_seq_port_info_t port information container on stack
        if (in) {
            snd_seq_port_info_alloca(&pinfo);
            if (amidi_data->vport < 0) {
                snd_seq_port_info_set_client( pinfo, 0 );
                snd_seq_port_info_set_port( pinfo, 0 );
                // Set the capability bits of a port_info container
                snd_seq_port_info_set_capability( pinfo, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE );
                snd_seq_port_info_set_type( pinfo, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION );
                snd_seq_port_info_set_midi_channels(pinfo, 16);
                #ifndef AVOID_TIMESTAMPING
                snd_seq_port_info_set_timestamping( pinfo, 1 );
                snd_seq_port_info_set_timestamp_real( pinfo, 1 );
                snd_seq_port_info_set_timestamp_queue( pinfo, amidi_data->queue_id );
                #endif
                snd_seq_port_info_set_name( pinfo,  port_name );
                amidi_data->vport = snd_seq_create_port( amidi_data->seq, pinfo );
                if (amidi_data->vport < 0) {
                    slog("Alsa MIDI in", "error creating an input port");
                    result = -1;
                }
                amidi_data->vport = snd_seq_port_info_get_port( pinfo );
            }
        } else {
            // Create a virtual output port
            if (amidi_data->vport < 0) {
               amidi_data->vport = snd_seq_create_simple_port(
                   amidi_data->seq,
                   port_name,
                   SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                   SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION
               );
               if (amidi_data->vport < 0) {
                   slog("Alsa MIDI out", "error creating output port.");
                   result = -1;
               }
           }
        }
        // Store a port at receiver or sender depending on a call options
        if (in) receiver.port = amidi_data->vport;
        else    sender.port   = amidi_data->vport;
        // Create a subscription for input or output port
        if (!amidi_data->subscription) {
            if (snd_seq_port_subscribe_malloc(&amidi_data->subscription) < 0) {
                if (in) {
                    slog("Alsa MIDI in", "error allocating port subscription.");
                    result = -1;
                } else {
                    snd_seq_port_subscribe_free(amidi_data->subscription);
                    slog("Alsa MIDI out", "error allocating port subscription.");
                    result = -1;
                }
            }
            // Set sender address of a port_subscribe container
            snd_seq_port_subscribe_set_sender(amidi_data->subscription, &sender);
            // Set destination address of a port_subscribe container
            snd_seq_port_subscribe_set_dest(amidi_data->subscription, &receiver);
            //
            if (!in) {
                // Set the time-update mode of a port_subscribe container
                snd_seq_port_subscribe_set_time_update(amidi_data->subscription, 1);
                // Set the real-time mode of a port_subscribe container
                snd_seq_port_subscribe_set_time_real(amidi_data->subscription, 1);
            }
            // Subscribe to a port, store ALSA subscription information in amidi_data->subscription
            if (snd_seq_subscribe_port(amidi_data->seq, amidi_data->subscription)) {
                if (in) {
                    snd_seq_port_subscribe_free(amidi_data->subscription);
                    amidi_data->subscription = 0;
                    slog("Alsa MIDI in", "error making port connection.");
                    result = -1;
                } else {
                    snd_seq_port_subscribe_free(amidi_data->subscription);
                    slog("ALSA MIDI out", "opening an output port: ALSA error making port connection.");
                    result = -1;
                }
            }
        } else {
            slog("Alsa MIDI", "subscription is already set.");
            result = -1;
        }
        // Start an input thread if it wasn't started yet
        if (in && input_data->do_input == false) {
            // Start the input queue
            #ifndef AVOID_TIMESTAMPING
            snd_seq_start_queue(amidi_data->seq, amidi_data->queue_id, NULL);
            snd_seq_drain_output(amidi_data->seq);
            #endif
            // Create and configure a thread attributes object
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
            // Mark input thread as started
            input_data->do_input = true;
            // Passing input data to a thread.
            // Create a thread
            int err = pthread_create(&amidi_data->thread, &attr, alsa_MIDI_handler, input_data);
            // Destroy thread attributes object
            pthread_attr_destroy(&attr);
            if (err) {
                snd_seq_unsubscribe_port(amidi_data->seq, amidi_data->subscription);
                snd_seq_port_subscribe_free(amidi_data->subscription);
                amidi_data->subscription = 0;
                input_data->do_input = false;
                slog("Alsa MIDI in", "error starting MIDI input thread.");
                exit(1);
            }
        }
        // Record that port is connected in a provided amidi_data instance
        amidi_data->port_connected = true;
    } while(0);
    return result;
}

/**
 * Closes input or output port, works for both virtual and not ports
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param input_data: :c:type:`MIDI_in_data` instance
 * :param mode: accepts :c:data:`SND_SEQ_OPEN_INPUT` or :c:data:`SND_SEQ_OPEN_OUTPUT`
 *
 * :since: v0.1
 */
void close_port(Alsa_MIDI_data * amidi_data, MIDI_in_data * input_data, int mode) {
     if (amidi_data->port_connected) {
         // Close output port
         if (mode == SND_SEQ_OPEN_OUTPUT) {
             snd_seq_unsubscribe_port(amidi_data->seq, amidi_data->subscription);
             snd_seq_port_subscribe_free(amidi_data->subscription);
             amidi_data->subscription = 0;
             amidi_data->port_connected = false;
        }
        // Close input port
        if (mode == SND_SEQ_OPEN_INPUT) {
            if (amidi_data->subscription) {
                snd_seq_unsubscribe_port(amidi_data->seq, amidi_data->subscription);
                snd_seq_port_subscribe_free(amidi_data->subscription);
                amidi_data->subscription = 0;
            }
            // Stop the input queue
            #ifndef AVOID_TIMESTAMPING
            snd_seq_stop_queue(amidi_data->seq, amidi_data->queue_id, NULL);
            snd_seq_drain_output(amidi_data->seq);
            #endif
            amidi_data->port_connected = false;
        }
    }
    // Stop thread to avoid triggering the callback,
    // while the port is intended to be closed
    if (mode == SND_SEQ_OPEN_INPUT && input_data->do_input) {
        input_data->do_input = false;
        int res = write(
            amidi_data->trigger_fds[1],
            &input_data->do_input,
            sizeof( input_data->do_input )
        );
        (void) res;
        if ( !pthread_equal( amidi_data->thread, amidi_data->dummy_thread_id ) )
            pthread_join( amidi_data->thread, NULL );
    }
}

/**
 * Destroys a MIDI output port: closes a port connection and performs a cleanup.
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param input_data: :c:type:`MIDI_in_data` instance
 *
 * :returns: **0** on success
 *
 * :since: v0.1
 */
int destroy_midi_output(Alsa_MIDI_data * amidi_data, MIDI_in_data * input_data) {
    int result = 0;
    // Close port connection if it exists
    close_port(amidi_data, input_data, SND_SEQ_OPEN_OUTPUT);
    // Do cleanup
    if (amidi_data->vport >= 0) {
        if (snd_seq_delete_port(amidi_data->seq, amidi_data->vport) != 0) result = -1;
    }
    if (amidi_data->coder) snd_midi_event_free(amidi_data->coder);
    if (amidi_data->buffer) free(amidi_data->buffer);
    if (snd_seq_close(amidi_data->seq) != 0) result = -1;
    free(amidi_data);
    return result;
}

/**
 * Destroys a MIDI input port:
 * closes a port connection, shuts the input thread down, performs cleanup / deallocations.
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param input_data: :c:type:`MIDI_in_data` instance
 *
 * :returns: **0** on success
 *
 * :since: v0.1
 */
int destroy_midi_input(Alsa_MIDI_data * amidi_data, MIDI_in_data * input_data) {
    int result = 0;
    // Close port connection if it exists
    close_port(amidi_data, input_data, SND_SEQ_OPEN_OUTPUT);
    // Shutdown the input thread.
    if ( input_data->do_input ) {
        input_data->do_input = false;
        int res = write(amidi_data->trigger_fds[1], &input_data->do_input, sizeof(input_data->do_input));
        (void) res;
        if ( !pthread_equal(amidi_data->thread, amidi_data->dummy_thread_id) ) pthread_join(amidi_data->thread, NULL);
    }
    // Do cleanup
    close(amidi_data->trigger_fds[0]);
    close(amidi_data->trigger_fds[1]);
    int port_delete_result;
    if (amidi_data->vport >= 0) {
        port_delete_result = snd_seq_delete_port(amidi_data->seq, amidi_data->vport);
        if (port_delete_result < 0) result = -1;
    }
    #ifndef AVOID_TIMESTAMPING
    int queue_free_result = snd_seq_free_queue(amidi_data->seq, amidi_data->queue_id);
    if (queue_free_result < 0) result = -1;
    #endif
    int seq_closing_result = snd_seq_close(amidi_data->seq);
    if (seq_closing_result < 0) result = -1;
    free(amidi_data);
    return result;
}

/**
 * Set the name of a port_info container in Alsa SEQ interface port information container,
 * update a port info value for an :c:member:`amidi_data.vport` value.
 *
 * :param amidi_data: Alsa_MIDI_data instance
 * :param port_name: a new name for Alsa seq port
 *
 * :since: v0.1
 */
void set_port_name(Alsa_MIDI_data *amidi_data, const char *port_name) {
    snd_seq_port_info_t *pinfo;
    snd_seq_port_info_alloca(&pinfo);
    snd_seq_get_port_info(amidi_data->seq, amidi_data->vport, pinfo);
    snd_seq_port_info_set_name(pinfo, port_name);
    snd_seq_set_port_info(amidi_data->seq, amidi_data->vport, pinfo);
}

/**
 * Set name for a :c:member:`amidi_data.seq` :c:type:`snd_seq_t` instance.
 *
 * :param amidi_data: :c:type:`Alsa_MIDI_data` instance
 * :param client_name: a new name for Alsa seq client
 *
 * :since: v0.1
 */
void set_client_name(Alsa_MIDI_data *amidi_data, const char *client_name) {
    snd_seq_set_client_name(amidi_data->seq, client_name);
}

/**
 * Allocates memory for :c:type:`MIDI_in_data` instance.
 * Assigns two queues: one for MIDI messages and one for errors.
 *
 * :param input_data: a double pointer used to allocate memory for a :c:type:`MIDI_in_data` instance
 *
 * :returns: **0** on success
 *
 * :since: v0.1
 */
int prepare_input_data_with_queues(MIDI_in_data ** input_data) {
    int result = 0;
    // Allocate memory for input_data
    * input_data = NULL;
    * input_data = malloc(sizeof(MIDI_in_data));
    if (input_data == NULL) slog("Start", "Unable to allocate memory for MIDI_in_data instance.");
    // Assign a queue for passing MIDI messages
    assign_midi_queue(*input_data);
    // Assign a queue for passing error messages
    assign_error_queue(*input_data);
    //
    return result;
}

/**
 * A wrapper function for a initializing a virtual output MIDI port.
 *
 * :param amidi_data: a double pointer to :c:type:`Alsa_MIDI_data` instance
 * :param port_config: an instance of port configuration: :c:type:`RMR_Port_config`
 *
 * :returns: **0** on success
 *
 * :since: v0.1
 */
int start_virtual_output_port(Alsa_MIDI_data **amidi_data, RMR_Port_config * port_config) {
    int result = 0;
    // Allocate Alsa_MIDI_data memory
    init_amidi_data_instance(amidi_data);
    // Fill :c:type:`Alsa_MIDI_data` instance
    init_amidi_data(*amidi_data, port_config->port_type);
    // Open an Alsa seq interface, assign it to :c:type:`Alsa_MIDI_data` instance
    init_seq(*amidi_data, port_config->client_name, port_config->port_type);
    // Open Alsa seq interface with a "virtual output" port
    start_virtual_output_seq(*amidi_data, port_config->port_name);
    //
    return result;
}

/**
 * A wrapper function for starting a non-virtual output port.
 *
 * :param amidi_data: a double pointer to :c:type:`Alsa_MIDI_data` instance
 * :param port_config: an instance of port configuration: :c:type:`RMR_Port_config`
 *
 * :returns: **0** on success
 *
 * :since: v0.1
 */
int start_output_port(Alsa_MIDI_data ** amidi_data, RMR_Port_config * port_config) {
    int result = 0;
    // Allocate Alsa_MIDI_data memory
    init_amidi_data_instance(amidi_data);
    // Fill :c:type:`Alsa_MIDI_data` instance
    init_amidi_data(*amidi_data, port_config->port_type);
    // Open an Alsa seq interface, assign it to :c:type:`Alsa_MIDI_data` instance
    init_seq(*amidi_data, port_config->client_name, port_config->port_type);
    start_output_seq(*amidi_data);
    //
    return result;
}

/**
 * A wrapper function for opening a virtual input port
 *
 * :param amidi_data: a double pointer to :c:type:`Alsa_MIDI_data` instance
 * :param port_config: an instance of port configuration: :c:type:`RMR_Port_config`
 *
 * :returns: **0** on success
 *
 * :since: v0.1
 */
int start_virtual_input_port(Alsa_MIDI_data **amidi_data, RMR_Port_config * port_config) {
    int result = 0;

    init_amidi_data_instance(amidi_data);
    // Fill :c:type:`Alsa_MIDI_data` struct instance
    init_amidi_data(*amidi_data, port_config->port_type);
    // Open an Alsa seq interface, assign it to :c:type:`Alsa_MIDI_data` instance
    init_seq(*amidi_data, port_config->client_name, port_config->port_type);
    start_input_seq(*amidi_data, port_config->queue_name, port_config);
    //
    return result;
}

/**
 * A wrapper function for starting a non-virtual input port.
 *
 * :param amidi_data: a double pointer to :c:type:`Alsa_MIDI_data` instance
 * :param port_config: an instance of port configuration: :c:type:`RMR_Port_config`
 *
 * :returns: **0** on success
 *
 * :since: v0.1
 */
int start_input_port(Alsa_MIDI_data **amidi_data, RMR_Port_config * port_config) {
    int result = 0;

    init_amidi_data_instance(amidi_data);
    // Fill Alsa_MIDI_data struct instance
    init_amidi_data(*amidi_data, port_config->port_type);
    // Open an Alsa seq interface, assign it to :c:type:`Alsa_MIDI_data` instance
    init_seq(*amidi_data, port_config->client_name, port_config->port_type);
    start_input_seq(*amidi_data, port_config->queue_name, port_config);

    return result;
}

/**
 * Fills :c:type:`RMR_Port_config` attributes.
 *
 * A port type is set first, then a queue tempo and ppq you can change.
 * "client_name", "port_name" and "queue_name" are set as "N/A" and then changed
 * to default values needed for a certain port type.
 *
 * :param port_config: an instance of port configuration: :c:type:`RMR_Port_config`
 * :param port_type: a port type for a sequencer, supports all values for :c:type:`mp_type_t`
 *
 * :returns: **0** on success
 *
 * :since: v0.1.3
 */
int reset_port_config(RMR_Port_config * port_config, mp_type_t port_type) {
    // Set port type in a port config
    port_config->port_type = port_type;
    // Queue tempo config, currently needed
    // for input and virtual input modes only
    port_config->queue_tempo = QUEUE_TEMPO;
    port_config->queue_ppq = QUEUE_STATUS_PPQ;
    // Configure port based on its type
    port_config->client_name = "N/A";
    port_config->port_name = "N/A";
    port_config->queue_name = "N/A";
    switch(port_type) {
        case MP_IN:
                port_config->client_name = "rmr input";
                port_config->queue_name = "rmr queue";
                break;
    	case MP_VIRTUAL_IN:
                port_config->client_name = "rmr virtual input";
                port_config->port_name = "rmr queue";
                break;
        case MP_OUT:
                port_config->client_name = "rmr output";
                break;
        case MP_VIRTUAL_OUT:
                port_config->client_name = "rmr virtual output";
                port_config->port_name = "rmr virtual output port";
                break;
        default:
                break;
    }
    return 0;
}

/**
 * Allocates memory for an instance of :c:type:`RMR_Port_config`,
 * sets default values for it
 *
 * :param port_config: an instance of port configuration: :c:type:`RMR_Port_config`
 * :param port_type: a port type for a sequencer, supports all values for :c:type:`mp_type_t`
 *
 * :returns: **0** on success
 *
 * :since: v0.1.3
 */
int setup_port_config(RMR_Port_config ** port_config, mp_type_t port_type) {
    * port_config = NULL;
    * port_config = malloc(sizeof(RMR_Port_config));
    int result = reset_port_config(*port_config, port_type);
    return result;
}

/**
 * Deallocates an instance of :c:type:`RMR_Port_config`
 *
 * :param port_config: an instance of port configuration (:c:type:`RMR_Port_config`)
 *
 * :returns: **0** on success
 *
 * :since: v0.1.3
 */
int destroy_port_config(RMR_Port_config * port_config) {
    int result = 0;
    free(port_config);
    return result;
}

/**
 * A wrapper for in, out, virtual in and virtual out MIDI port initialization.
 * TODO add Port_config input.
 *
 * :param amidi_data: a double pointer to :c:type:`Alsa_MIDI_data` instance
 * :param port_config: an instance of port configuration: :c:type:`RMR_Port_config`
 *
 * :returns: **0** on success
 *
 * :since: v0.1
 */
int start_port(Alsa_MIDI_data **amidi_data, RMR_Port_config * port_config) {
    int result = 0;
    switch(port_config->port_type) {
        case MP_IN:
                result = start_input_port(amidi_data, port_config);
                break;
    	case MP_VIRTUAL_IN:
                result = start_virtual_input_port(amidi_data, port_config);
                break;
        case MP_OUT:
                result = start_output_port(amidi_data, port_config);
                break;
        case MP_VIRTUAL_OUT:
                result = start_virtual_output_port(amidi_data, port_config);
                break;
        default:
                result = -1;
                break;
    }
    return result;
}

/**
 * Finds a complete port name, including both **client info** and **port info**.
 * A rewrite of both RtMIDI's getPortName functions.
 *
 * :param port_name: a const char pointer pointing to a string to be filled
 * :param port_number: a number of a port to look for
 * :param amidi_data: a double pointer to :c:type:`Alsa_MIDI_data` instance
 * :param in: defines if a function is looking for "SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ" or
 *            "SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE" capabilities
 *
 * :returns: **0** on success, **-1** when a MIDI port is not found.
 *
 * :since: v0.1
 */
int get_full_port_name(const char * port_name, unsigned int port_number, Alsa_MIDI_data * amidi_data, bool in) {
  int result = 0;
  int32_t port_mode;

  // Client info container
  snd_seq_client_info_t *cinfo;
  // Port info container
  snd_seq_port_info_t *pinfo;
  // Allocate memory for *cinfo and *pinfo.
  snd_seq_client_info_alloca(&cinfo);
  snd_seq_port_info_alloca(&pinfo);

  static char string_name[512];

  if (in) port_mode = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
  else    port_mode = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;

  unsigned int port_id = port_info(amidi_data->seq, pinfo, port_mode, (int) port_number);

  if (port_id) {
    // Get client id of a port_info container
    int cnum = snd_seq_port_info_get_client(pinfo);
    // Obtain the information of the given client
    snd_seq_get_any_client_info(amidi_data->seq, cnum, cinfo);
    // Generate a full port name
    sprintf(
        string_name,
        "%s:%s %d:%d",
        snd_seq_client_info_get_name(cinfo),
        snd_seq_port_info_get_name(pinfo),
        // These lines added to make sure devices are listed
        // with full portnames added to ensure individual device names
        snd_seq_port_info_get_client(pinfo),
        snd_seq_port_info_get_port(pinfo)
    );
    port_name = string_name;
    return result;
  }

  // Port match was not found
  result = -1;
  return result;
}
