#include "fixtures/assert.h"
#include "fixtures/stdcapt.h"

static struct capt capt;

int
foo() {
    errno = 1;
    ERROR("foo");
    return ERR;
}

TEST_CASE void
test_stdcapt(struct test *t) {
    STDCAPT_ERR(capt);
    EQI(foo(), ERR);
    EQERR(capt, "test_stdcapt: foo: Operation not permitted" N);
}

int
main() {
    struct test *t = TEST_BEGIN(LL_ERROR);
    test_stdcapt(t);
    return TEST_CLEAN(t);
}
