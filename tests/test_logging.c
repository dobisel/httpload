#include "logging.h"
#include "fixtures/assert.h"
#include "fixtures/capture.h"
#include <stdlib.h>

#define PROG    "httploadc"

int
monkeymain(int argc, char **argv) {
    ERROR("e");
    WARN("w");
    INFO("i");
    DEBUG("d");
    return EXIT_SUCCESS;
}

#define fcapt(...) fcapture(monkeymain, PROG, 0, NULL, ## __VA_ARGS__)

void
test_logging_verbosity() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    /* Debug */
    log_setlevel(LL_DEBUG);
    errno = 0;
    status = fcapt(out, err);
    eqint(0, status);
    eqstr("e" N, err);
    eqstr("Warning: w" N "Info: i" N "Debug: [monkeymain:13] d" N, out);

    /* Info */
    log_setlevel(LL_INFO);
    status = fcapt(out, err);
    eqint(0, status);
    eqstr("e" N, err);
    eqstr("Warning: w" N "Info: i" N, out);

    /* Warning */
    log_setlevel(LL_WARN);
    status = fcapt(out, err);
    eqint(0, status);
    eqstr("e" N, err);
    eqstr("Warning: w" N, out);

    /* ERROR */
    log_setlevel(LL_ERROR);
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
