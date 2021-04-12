#include "testing.h"
#include "logging.h"
#include "server_cli.h"
#include "fixtures/capture.h"

#define PROG    "httploads"
#define fcapttimeout(t, ...) \
    fcapture_timeout((t), servercli_run, PROG, ## __VA_ARGS__)
#define fcapt(...) fcapttimeout(0, ## __VA_ARGS__)
#define fcapttime(...) fcapttimeout(1, ## __VA_ARGS__)

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
test_fork() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = fcapttime(1, (char *[]) { "-c2" }, out, err);
    eqint(0, status);

    status = fcapttime(1, (char *[]) { "-V" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION N, out);
}

int
main() {
    log_setlevel(LL_DEBUG);
    test_version();
    test_fork();
    return EXIT_SUCCESS;
}
