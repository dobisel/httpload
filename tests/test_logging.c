#include "logging.h"
#include "fixtures/assert.h"
#include "fixtures/capture.h"
#include <stdlib.h>

#define PROG    "httploadc"

int
monkeymain(int argc, char **argv) {
    WARN("w");
    INFO("i");
    DBUG("d");
    errno = 1;
    ERRX("e");
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
    eqint(1, status);
    eqstr("test_logging: w: Success" N
          "test_logging: e: Operation not permitted" N, err);
    eqstr("i" N "012:monkeymain -- d" N, out);

    /* Info */
    log_setlevel(LL_INFO);
    status = fcapt(out, err);
    eqint(1, status);
    eqstr("test_logging: w: Success" N
          "test_logging: e: Operation not permitted" N, err);
    eqstr("i" N, out);

    /* Warning */
    log_setlevel(LL_WARN);
    status = fcapt(out, err);
    eqint(1, status);
    eqstr("test_logging: w: Success" N
          "test_logging: e: Operation not permitted" N, err);
    eqstr("", out);

    /* ERROR */
    log_setlevel(LL_ERROR);
    status = fcapt(out, err);
    eqint(1, status);
    eqstr("test_logging: e: Operation not permitted" N, err);
    eqstr("", out);
}

int
main() {
    test_logging_verbosity();
    return EXIT_SUCCESS;
}
