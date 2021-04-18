#include "logging.h"
#include "client_cli.h"
#include "fixtures/assert.h"
#include "fixtures/capture.h"

#define PROG    "httploadc"
#define fcapt(...) fcapture(clientcli_run, PROG, ## __VA_ARGS__)

static void
test_version(struct test *t) {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = fcapt(1, (char *[]) { "--version" }, out, err);
    EQI(status, 0);
    EQS(out, HTTPLOAD_VERSION N);

    status = fcapt(1, (char *[]) { "-V" }, out, err);
    EQI(status, 0);
    EQS(out, HTTPLOAD_VERSION N);
}

static void
test_verbosity(struct test *t) {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = fcapt(2, (char *[]) { "-v", "1" }, out, err);
    EQI(status, 0);
    EQS(out, "");
    EQS(err, "");

    status = fcapt(2, (char *[]) { "-v", "2" }, out, err);
    EQI(status, 0);
    EQS(out, "");
    EQS(err, "");

    status = fcapt(2, (char *[]) { "-v", "3" }, out, err);
    EQI(status, 0);
    EQS(out, "");
    EQS(err, "");

    status = fcapt(2, (char *[]) { "-v", "4" }, out, err);
    EQI(status, 0);
    EQS(out, "");
    EQS(err, "");

    status = fcapt(2, (char *[]) { "-v", "5" }, out, err);
    EQI(status, 64);
    EQS(out, "");
    EQNS(29, err, PROG ": Invalid verbosity level: 5");
}

static void
test_invalidargument(struct test *t) {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    /* Invalid optional argument. */
    status = fcapt(2, (char *[]) { "--invalidargument", "0" }, out, err);
    EQI(status, 64);
    EQS(out, "");
    EQNS(49, err, PROG ": unrecognized option '--invalidargument'");

    /* Extra positional arguments. */
    status = fcapt(3, (char *[]) { "foo", "bar", "baz" }, out, err);
    EQI(status, 64);
    EQS(out, "");
    EQNS(29, err, PROG ": Too many arguments");
}

int
main() {
    struct test t;

    SETUP(&t);
    test_version(&t);
    test_verbosity(&t);
    test_invalidargument(&t);
    return TEARDOWN(&t);
}
