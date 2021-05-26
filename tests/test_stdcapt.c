#include "common.h"
#include "logging.h"
#include "helpers.h"
#include "fixtures/assert.h"
#include "fixtures/stdcapt.h"

#include <unistd.h>

static struct capt capt;
static struct test *t;

int
foo() {
    errno = 1;
    ERROR("foo");
    return ERR;
}

static void
test_stdcapt() {
    capt_start(&capt, STDCAPT_NO_OUT);
    EQI(foo(), ERR);
    capt_restore(&capt);
    EQS(capt.err, "test_stdcapt: foo: Operation not permitted\n");
}

int
main() {
    static struct test test;

    log_setlevel(LL_DEBUG);
    t = &test;
    SETUP(t);
    test_stdcapt();
    return TEARDOWN(t);
}
