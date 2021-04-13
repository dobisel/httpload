#include "testing.h"
#include "logging.h"
#include "server_cli.h"
#include "fixtures/capture.h"
#include "fixtures/curl.h"

#define T   2
#define PROG    "httploads"
#define fcapttimeout(t, ...) \
    fcapture_timeout((t), servercli_run, PROG, ## __VA_ARGS__)
#define fcapt(...) fcapttimeout(0, ## __VA_ARGS__)
#define fcapttime(...) fcapttimeout(T, ## __VA_ARGS__)

static void
test_version() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = fcapt(1, (char *[]) { "--version" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION N, out);
    eqstr("", err);

    status = fcapt(1, (char *[]) { "-V" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION N, out);
    eqstr("", err);
}

static void
test_fork() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = fcapttime(1, (char *[]) { "-c2" }, out, err);
    eqint(0, status);
    eqstr("", err);

    status = fcapttime(1, (char *[]) { "-V" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION N, out);
    eqstr("", err);
}

static void
test_invalidargument() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    /* Invalid optional argument. */
    status = fcapttime(2, (char *[]) { "--invalidargument", "0" }, out, err);
    neqint(0, status);
    eqstr("", out);
    eqnstr(PROG ": unrecognized option '--invalidargument'", err, 49);

    /* Extra positional arguments. */
    status = fcapttime(3, (char *[]) { "foo", "bar", "baz" }, out, err);
    neqint(0, status);
    eqstr("", out);
    eqnstr(PROG ": Too many arguments", err, 29);

    /* Invalid fork counts. */
    status = fcapttime(1, (char *[]) { "-c0" }, out, err);
    neqint(0, status);
    eqstr("", out);
    eqnstr("Invalid number of forks", err, 23);
}

static void
test_port() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };

    eqint(0, fcapttime(1, (char *[]) { "-p8080" }, out, err));
    eqstr("Info: Listening on port: 8080" N, out);
    eqstr("", err);
}

int
main() {
    log_setlevel(LL_DEBUG);
    test_version();
    test_fork();
    test_invalidargument();
    test_port();
    return EXIT_SUCCESS;
}
