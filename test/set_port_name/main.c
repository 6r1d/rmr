#include "midi/midi_handling.h"

Alsa_MIDI_data * amidi_data;

int main() {
    // Start a virtual out port to init data structures
    start_port(&amidi_data, MP_VIRTUAL_OUT);

    // Set a new port name
    set_port_name(amidi_data, "New port name");

    // Destroy a virtual port
    if (destroy_midi_output(amidi_data, NULL) != 0) slog("destructor", "destructor error");

    // Exit without an error
    return 0;
}
