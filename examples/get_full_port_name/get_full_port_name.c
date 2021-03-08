/*
 * This example finds and identifies a virtual output port.
 */

#include <stdbool.h>
#include "util/output_handling.h"
// Main RMR header file
#include "midi/midi_handling.h"

Alsa_MIDI_data * data;
MIDI_in_data * input_data;
MIDI_port * current_midi_port;
RMR_Port_config * port_config;

char * port_name;

#define PORT_NAME_MAX_LENGTH 512
// MP_VIRTUAL_OUT for virtual output, MP_VIRTUAL_IN for virtual input.
// Non-virtual ports are accepted the same way.
mp_type_t expected_port_type = MP_VIRTUAL_OUT;

int main() {
    port_name = (char*)calloc(PORT_NAME_MAX_LENGTH, sizeof(char));

    // Create a port configuration with default values
    setup_port_config(&port_config, MP_IN);
    // Start a port with a provided configruation
    start_port(&data, port_config);

    // Allocate memory for a MIDI_port instance
    init_midi_port(&current_midi_port);

    // Count the MIDI ports,
    // open if a port containing "Synth" is available
    if (find_midi_port(data, current_midi_port, expected_port_type, "rmr") > 0) {
        get_full_port_name(port_name, current_midi_port->id, expected_port_type, data);
        printf("%s\n", port_name);
    }

    // Free the memory used by port_name
    free(port_name);

    // Close a MIDI input port,
    // shutdown the input thread,
    // do cleanup
    destroy_midi_input(data, input_data);

    // Destroy a port configuration
    destroy_port_config(port_config);

    // Exit without an error
    return 0;
}
