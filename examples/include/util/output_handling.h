#include <stdio.h>

void print_midi_msg_buf(unsigned char * buf, long count) {
    long byte_id;
    for (byte_id = 0; byte_id < count; byte_id++) {
        printf("%02x ", buf[byte_id]);
    }
    printf(" | ");
    for (byte_id = 0; byte_id < count; byte_id++) {
        printf("%d ", buf[byte_id]);
    }
    printf("\n");
    fflush( stdout );
}
