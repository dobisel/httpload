#include "logging.h"
#include "testing.h"
#include "fixtures/httpdmock.h"

void
test_single_packet() {
    struct httpdmock m;

    httpdmock_start(&m);
    eqint(200, httpdmock_get(&m));
    eqstr("Hello HTTPLOAD!", m.out);
    eqstr("", m.err);
    httpdmock_stop(&m);
}

int
main() {
    log_setlevel(LL_DEBUG);
    test_single_packet();
    return 0;
}
