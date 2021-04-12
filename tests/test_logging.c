#include "logging.h"
#include "testing.h"
#include "fixtures/capture.h"

#include <stdlib.h>

int
monkeymain(int argc, char **argv) {
    ERROR("e");
    WARN("w");
    INFO("i");
    DEBUG("d");
    return EXIT_SUCCESS;
}

#define PROG    "monkey"
#define fcapt(...) fcapture(monkeymain, PROG, 0, NULL, ## __VA_ARGS__)

void
test_logging_verbosity() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    /* Debug */
    log_setlevel(LOG_DEBUG);
    status = fcapt(out, err);
    eqint(0, status);
    eqstr("e" N, err);
    eqstr("Warning: w" N "Info: i" N "Debug: [monkeymain:12] d" N, out);

    /* Info */
    log_setlevel(LOG_INFO);
    status = fcapt(out, err);
    eqint(0, status);
    eqstr("e" N, err);
    eqstr("Warning: w" N "Info: i" N, out);

    /* Warning */
    log_setlevel(LOG_WARN);
    status = fcapt(out, err);
    eqint(0, status);
    eqstr("e" N, err);
    eqstr("Warning: w" N, out);

    /* ERROR */
    log_setlevel(LOG_ERROR);
    status = fcapt(out, err);
    eqint(0, status);
    eqstr("e" N, err);
    eqstr("", out);
}

int
main() {
    test_logging_verbosity();
    return EXIT_SUCCESS;
}
