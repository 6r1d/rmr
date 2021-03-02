#include <stdio.h>

/**
 * Logging-related functions
 */

/**
 * A function to display generic status messages
 *
 * :param section: a section of a status message to display
 * :param message: a message to be displayed
 *
 * :since: v0.1
 */
void slog(const char *section, const char *message) {
    printf("\033[1m\033[37m[%s]\033[0m: %s\n", section, message);
}
