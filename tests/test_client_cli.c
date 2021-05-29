#include "client_cli.h"
#include "fixtures/assert.h"
#include "fixtures/pcapt.h"

static struct pcapt p = {.prog = "httploadc" };

#define CCAPTW0(...) PCAPTW0(&p, clientcli_run, __VA_ARGS__)

TEST_CASE void
test_version(struct test *t) {
    EQI(CCAPTW0("--version"), 0);
    EQS(p.out, HTTPLOAD_VERSION N);

    EQI(CCAPTW0("-V"), 0);
    EQS(p.out, HTTPLOAD_VERSION N);
}

TEST_CASE void
test_verbosity(struct test *t) {
    EQI(CCAPTW0("-v", "1"), 0);
    EQS(p.out, "");
    EQS(p.err, "");

    EQI(CCAPTW0("-v1"), 0);
    EQS(p.out, "");
    EQS(p.err, "");

    EQI(CCAPTW0("-v2"), 0);
    EQS(p.out, "");
    EQS(p.err, "");

    EQI(CCAPTW0("-v3"), 0);
    EQS(p.out, "");
    EQS(p.err, "");

    EQI(CCAPTW0("-v4"), 0);
    EQS(p.out, "");
    EQS(p.err, "");

    EQI(CCAPTW0("-v5"), 64);
    EQS(p.out, "");
    EQNS(29, p.err, "httploadc: Invalid verbosity level: 5");
}

TEST_CASE void
test_invalidargument(struct test *t) {
    /* Invalid optional argument. */
    EQI(CCAPTW0("--invalidargument", "0"), 64);
    EQS(p.out, "");
    EQNS(49, p.err, "httploadc: unrecognized option '--invalidargument'");

    /* Extra positional arguments. */
    EQI(CCAPTW0("foo", "bar", "baz", "0"), 64);
    EQS(p.out, "");
    EQNS(29, p.err, "httploadc: Too many arguments");
}

int
main() {
    struct test *t = TEST_BEGIN(LL_ERROR);
    test_version(t);
    test_verbosity(t);
    test_invalidargument(t);
    return TEST_CLEAN(t);
}
