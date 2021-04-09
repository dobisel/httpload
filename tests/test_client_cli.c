#include "testing.h"
#include "logging.h"
#include "client_cli.h"
#include "fixture_cli.h"


#define PROG    "httpload"
#define CAPTURE(...) cli_capture(clientcli_run, PROG, ## __VA_ARGS__)


void
test_version() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = CAPTURE(1, (char *[]) { "--version" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION CR, out);

    status = CAPTURE(1, (char *[]) { "-V" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION CR, out);
}


void
test_verbosity() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = CAPTURE(2, (char *[]) { "-v", "0" }, out, err);
    eqint(0, status);
    eqstr("", out);
    eqstr("", err);
}

void
test_invalidargument() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    /* Invalid optional argument. */
    status = CAPTURE(2, (char *[]) { "--invalidargument", "0" }, out, err);
    eqint(EINVAL, status);
    eqstr("", out);
    eqnstr("httpload: unrecognized option '--invalidargument'", err, 49);

    /* Invalid positional arguments. */
    status = CAPTURE(3, (char *[]) { "foo", "bar", "baz" }, out, err);
    eqint(EINVAL, status);
    eqstr("", out);
    eqnstr("httpload: Too many arguments", err, 28);
}


int
main() {
    log_setlevel(LOG_DEBUG);
    test_version();
    test_verbosity();
    test_invalidargument();
    return EXIT_SUCCESS;
}
