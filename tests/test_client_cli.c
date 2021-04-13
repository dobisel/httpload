#include "logging.h"
#include "client_cli.h"
#include "fixtures/assert.h"
#include "fixtures/capture.h"

#define PROG    "httploadc"
#define fcapt(...) fcapture(clientcli_run, PROG, ## __VA_ARGS__)

static void
test_version() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = fcapt(1, (char *[]) { "--version" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION N, out);

    status = fcapt(1, (char *[]) { "-V" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION N, out);
}

static void
test_verbosity() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = fcapt(2, (char *[]) { "-v", "1" }, out, err);
    eqint(0, status);
    eqstr("", out);
    eqstr("", err);

    status = fcapt(2, (char *[]) { "-v", "2" }, out, err);
    eqint(0, status);
    eqstr("", out);
    eqstr("", err);

    status = fcapt(2, (char *[]) { "-v", "3" }, out, err);
    eqint(0, status);
    eqstr("", out);
    eqstr("", err);

    status = fcapt(2, (char *[]) { "-v", "4" }, out, err);
    eqint(0, status);
    eqstr("", out);
    eqstr("", err);

    status = fcapt(2, (char *[]) { "-v", "5" }, out, err);
    eqint(64, status);
    eqstr("", out);
    eqnstr(PROG ": Invalid verbosity level: 5", err, 29);
}

static void
test_invalidargument() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    /* Invalid optional argument. */
    status = fcapt(2, (char *[]) { "--invalidargument", "0" }, out, err);
    eqint(64, status);
    eqstr("", out);
    eqnstr(PROG ": unrecognized option '--invalidargument'", err, 49);

    /* Extra positional arguments. */
    status = fcapt(3, (char *[]) { "foo", "bar", "baz" }, out, err);
    eqint(64, status);
    eqstr("", out);
    eqnstr(PROG ": Too many arguments", err, 29);
}

int
main() {
    log_setlevel(LL_DEBUG);
    test_version();
    test_verbosity();
    test_invalidargument();
    return EXIT_SUCCESS;
}
