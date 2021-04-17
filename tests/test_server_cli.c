#include "logging.h"
#include "server_cli.h"
#include "fixtures/assert.h"
#include "fixtures/capture.h"
#include "fixtures/curl.h"

#define T   2
#define PROG    "httploads"
#define fcapttimeout(t, ...) fcapture_timeout((t), servercli_run, PROG, ## __VA_ARGS__)
#define fcapt(...) fcapttimeout(0, ## __VA_ARGS__)

static void
test_version(struct test *t) {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = fcapt(1, (char *[]) { "--version" }, out, err);
    EQI(status, 0);
    EQS(out, HTTPLOAD_VERSION N);
    EQS(err, "");

    status = fcapt(1, (char *[]) { "-V" }, out, err);
    EQI(status, 0);
    EQS(out, HTTPLOAD_VERSION N);
    EQS(err, "");
}

static void
test_fork(struct test *t) {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = fcapt(2, (char *[]) { "--dry", "-c2" }, out, err);
    EQI(status, 0);
    EQS(err, "");

    status = fcapt(2, (char *[]) { "--dry", "-V" }, out, err);
    EQI(status, 0);
    EQS(out, HTTPLOAD_VERSION N);
    EQS(err, "");
}

static void
test_invalidargument(struct test *t) {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    /* Invalid optional argument. */
    status =
        fcapt(3, (char *[]) { "--dry", "--invalidargument", "0" }, out, err);
    NEQI(status, 0);
    EQS(out, "");
    EQNS(49, err, PROG ": unrecognized option '--invalidargument'");

    /* Extra positional arguments. */
    status = fcapt(4, (char *[]) { "--dry", "foo", "bar", "baz" }, out, err);
    NEQI(status, 0);
    EQS(out, "");
    EQNS(29, err, PROG ": Too many arguments");

    /* Invalid fork counts. */
    status = fcapt(2, (char *[]) { "--dry", "-c0" }, out, err);
    NEQI(status, 0);
    EQS(out, "");
    EQNS(42, err, "test_server_cli: Invalid number of forks: 0");
}

static void
test_port(struct test *t) {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };

    EQI(fcapt(2, (char *[]) { "--dry", "-b888" }, out, err), 0);
    EQS(err, "");
    EQS(out, "forks:\t\t1" N "bind:\t\t888" N "verbosity:\tDebug(4)" N);
}

int
main() {
    struct test t;
    SETUP(&t);
    test_version(&t);
    test_fork(&t);
    test_invalidargument(&t);
    test_port(&t);
    return TEARDOWN(&t);
}
