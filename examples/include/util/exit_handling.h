// Using "signals" library to catch ctrl-c / SIGINT event.
//
// SO topic:
// https://stackoverflow.com/questions/4217037/

#include <signal.h>

static volatile int keep_process_running = 0;

void sigint_handler(int dummy) {
    keep_process_running = 0;
}


