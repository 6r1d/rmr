/**
 * MIDI-related type definitions
 */

/**
 * A function definition for processing MIDI callbacks
 */
typedef void ( * MIDI_callback ) (double timestamp, unsigned char * buf, long count, void * user_data);

/**
 * MIDI port type
 */
typedef enum {
  /** Virtual input mode */
  MP_VIRTUAL_IN,
  /** Virtual output mode */
  MP_VIRTUAL_OUT,
  /** Input mode */
  MP_IN,
  /** Output mode */
  MP_OUT
} mp_type_t;
