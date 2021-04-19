#include "logging.h"
#include "fixtures/assert.h"
#include "fixtures/pcapt.h"
#include <stdlib.h>

static struct pcapt p = {.prog = "logmock"};

int
monkeymain(int argc, char **argv) {
    WARN("w");
    INFO("i");
    DBUG("d");
    errno = 1;
    ERRX("e");
}

#define LCAPTW0() PCAPTW0(&p, monkeymain)

void
test_logging_verbosity(struct test *t) {
    /* Debug */
    log_setlevel(LL_DEBUG);
    errno = 0;
    EQI(LCAPTW0(), 1);
    EQS(p.err,
        "test_logging: w: Success" N
        "test_logging: e: Operation not permitted" N);
    EQS(p.out, "i" N "012:monkeymain -- d" N);

    /* Info */
    log_setlevel(LL_INFO);
    EQI(LCAPTW0(), 1);
    EQS(p.err,
        "test_logging: w: Success" N
        "test_logging: e: Operation not permitted" N);
    EQS(p.out, "i" N);

    /* Warning */
    log_setlevel(LL_WARN);
    EQI(LCAPTW0(), 1);
    EQS(p.err,
        "test_logging: w: Success" N
        "test_logging: e: Operation not permitted" N);
    EQS(p.out, "");

    /* ERROR */
    log_setlevel(LL_ERROR);
    EQI(LCAPTW0(), 1);
    EQS(p.err, "test_logging: e: Operation not permitted" N);
    EQS(p.out, "");
}

int
main() {
    struct test t;

    log_setlevel(LL_DEBUG);
    SETUP(&t);
    test_logging_verbosity(&t);
    return TEARDOWN(&t);
}
