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
test_logging_verbosity(struct test *t) {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    /* Debug */
    log_setlevel(LL_DEBUG);
    errno = 0;
    status = fcapt(out, err);
    EQI(status, 1);
    EQS(err, 
        "test_logging: w: Success" N 
        "test_logging: e: Operation not permitted" N);
    EQS(out, "i" N "012:monkeymain -- d" N);

    /* Info */
    log_setlevel(LL_INFO);
    status = fcapt(out, err);
    EQI(status, 1);
    EQS(err, "test_logging: w: Success" N "test_logging: e: Operation not permitted" N);
    EQS(out, "i" N);

    /* Warning */
    log_setlevel(LL_WARN);
    status = fcapt(out, err);
    EQI(status, 1);
    EQS(err, "test_logging: w: Success" N "test_logging: e: Operation not permitted" N);
    EQS(out, "");

    /* ERROR */
    log_setlevel(LL_ERROR);
    status = fcapt(out, err);
    EQI(status, 1);
    EQS(err, "test_logging: e: Operation not permitted" N);
    EQS(out, "");
}

int
main() {
    struct test t;
    SETUP(&t);
    test_logging_verbosity(&t);
    return TEARDOWN(&t);
}
