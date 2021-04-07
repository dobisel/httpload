#include "logging.h"
#include "httpdmock.h"
#include "testing.h"

#include <unistd.h>


void test_single_packet() {
    struct httpdmock m = {.port = 9090, .forks = 1};
    log_setlevel(LOG_DEBUG);
    int err = httpdmock_fork(&m);
    if (err) {
        FATAL("Cannot start http mock server");
    }
    INFO("Listening on port: %d", m.port);
    sleep(1);
    
    eqint(0, httpdmock_join(&m));
}

int
main() {
    test_single_packet();
    return 0;
}
