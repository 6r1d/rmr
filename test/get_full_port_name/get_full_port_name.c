#include <stdbool.h>
// Main RMR header file
#include "midi/midi_handling.h"

Alsa_MIDI_data * input_amidi_data;

#define PORT_NAME_MAX_LENGTH 512

Alsa_MIDI_data * amidi_data;
RMR_Port_config * port_config;

void scan_for_virtual_port()
{
    // MP_VIRTUAL_OUT for virtual output, MP_VIRTUAL_IN for virtual input.
    // Non-virtual ports are accepted the same way.
    mp_type_t expected_port_type = MP_VIRTUAL_OUT;

    char * port_name;
    port_name = (char*)calloc(PORT_NAME_MAX_LENGTH, sizeof(char));

    RMR_Port_config * port_config;

    // Create a port configuration with default values
    setup_port_config(&port_config, MP_IN);
    // Start a port with a provided configruation
    start_port(&input_amidi_data, port_config);

    MIDI_port * current_midi_port;
    // Allocate memory for a MIDI_port instance
    init_midi_port(&current_midi_port);

    // Count the MIDI ports,
    // open if a port containing "Synth" is available
    if (find_midi_port(input_amidi_data, current_midi_port, expected_port_type, "rmr") > 0) {
        get_full_port_name(port_name, current_midi_port->id, expected_port_type, input_amidi_data);
        printf("%s\n", port_name);
    }

    // Free the memory used by port_name
    free(port_name);

    // Close a MIDI input port,
    // shutdown the input thread,
    // do cleanup
    MIDI_in_data * input_data;
    // Allocate a MIDI_in_data instance, assign a
    // MIDI message queue and an error queue
    prepare_input_data_with_queues(&input_data);

    destroy_midi_input(input_amidi_data, input_data);

    // Destroy a port configuration
    destroy_port_config(port_config);
}

void create_virtual_port()
{
    // Create a port configuration with default values
    setup_port_config(&port_config, MP_VIRTUAL_OUT);
    // Start a port with a provided configruation
    start_port(&amidi_data, port_config);
}

void destroy_virtual_port()
{
    // Destroy a MIDI output port:
    // close a port connection and perform a cleanup.
    if (destroy_midi_output(amidi_data, NULL) != 0) slog("destructor", "destructor error");
    // Destroy a port configuration
    destroy_port_config(port_config);
}

int main()
{
    // TODO that'll be much nicer if
    // I could've tested both virtual input
    // and virtual output
    create_virtual_port();
    scan_for_virtual_port();
    destroy_virtual_port();

    // Exit without an error
    return 0;
}
