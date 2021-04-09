#include "logging.h"
#include "server.h"
#include "testing.h"

#include <unistd.h>


void test_single_packet() {
    struct httpd m = {.port = 9090, .forks = 1};
    log_setlevel(LOG_DEBUG);
    int err = httpd_fork(&m);
    if (err) {
        FATAL("Cannot start http mock server");
    }
    INFO("Listening on port: %d", m.port);

    // TODO: do tests

    sleep(1);
    eqint(0, httpd_join(&m));
}

int
main() {
    test_single_packet();
    return 0;
}
