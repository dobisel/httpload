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
    STDCAPT_ERR(capt);
    EQI(foo(), ERR);
    EQERR(capt, "test_stdcapt: foo: Operation not permitted" N);
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
