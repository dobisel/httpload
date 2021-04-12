#include "testing.h"
#include "logging.h"
#include "server_cli.h"
#include "fixtures/capture.h"

#define PROG    "httploads"
#define fcapt(...) fcapture(servercli_run, PROG, ## __VA_ARGS__)

void
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

int
main() {
    log_setlevel(LOG_DEBUG);
    test_version();
    return EXIT_SUCCESS;
}
