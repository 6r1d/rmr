#include <glib.h>

/**
 * Helper functions
 */

/**
 * Retrieve a last byte in a bytearray
 *
 * :param bytearray: a GLib :c:type:`GArray` instance to extract the last byte from
 *
 * :returns: a last character of GArray
 *
 * :since: v0.1
 */
unsigned char get_last_bytearray_byte(GArray * bytearray) {
    unsigned char result = 0;
    if (bytearray->len)
        result = g_array_index(bytearray, unsigned char, bytearray->len - 1);
    return result;
}
