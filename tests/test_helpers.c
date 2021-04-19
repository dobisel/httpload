#include "common.h"
#include "logging.h"
#include "fixtures/assert.h"
#include "ev_common.h"

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

/* third-party */
#include <mimick.h>

static struct test *t;

/**********************************
 * enable_nonblocking()
 *********************************/

MMK_DEFINE(fcntl_mock_t, int, int, int, int);

#define FCNTLMOCK fcntl_mock(mmk_eq(int, 888), mmk_any(int), mmk_any(int))

static void
test_enable_nonblocking() {
    int ret;
    fcntl_mock_t fcntl_mock = mmk_mock("fcntl@self", fcntl_mock_t);

    /* When fcntl raise error. */
    ret = ERR;
    MMK_WHEN(FCNTLMOCK, &ret, ENOENT);
    enable_nonblocking(888);
    MMKOK(FCNTLMOCK, 1);

    MMK_RESET(fcntl);
}

int
main() {
    static struct test test;

    log_setlevel(LL_DEBUG);
    t = &test;
    SETUP(t);
    test_enable_nonblocking();
    return TEARDOWN(t);
}
