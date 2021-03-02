/*
 * Time measurement functions
 */

#include <sys/time.h>

// Returns the milliseconds since Epoch
double millis() {
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    return (cur_time.tv_sec * 1000.0) + cur_time.tv_usec / 1000.0;
}
