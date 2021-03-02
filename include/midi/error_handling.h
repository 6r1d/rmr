/**
 * Error handling
 */

#define ERROR_MSG_ETYPE_SIZE 5
#define ERROR_MSG_TEXT_SIZE 255

/**
 * A struct to hold an error message.
 *
 * :since: v0.1
 */
typedef struct error_message {
    /** Integer value to differentiate between possible error types */
    char error_type[ERROR_MSG_ETYPE_SIZE];
    /** Current error message string */
    char message[ERROR_MSG_TEXT_SIZE];
} error_message;

/**
 * Free the memory used by an error message
 *
 * :param msg: a message to deallocate
 *
 * :since: v0.1
 */
void free_error_message(error_message * msg) {
    free(msg);
}

/**
 * A function to display error messages.
 * TODO: integrate error classes support, currently the function doesn't differentiate
 * between system and value errors.
 *
 * :param err_id: an `ID <../errors.html>`_ of an error to classify it by
 * :param section: where error happened
 * :param message: error message to send
 *
 * :since: v0.1
 */
void serr(const char *err_id, const char *section, const char *message) {
    printf("\033[1m\033[37m[%s | %s]\033[0m: %s\n", err_id, section, message);
}
