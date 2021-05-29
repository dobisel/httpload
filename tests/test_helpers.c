#include "common.h"
#include "logging.h"
#include "fixtures/assert.h"
#include "ev_common.h"

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

/* third-party */
#include <mimick.h>

/**********************************
 * enable_nonblocking()
 *********************************/

MMK_DEFINE(fcntl_mock_t, int, int, int, int);

#define FCNTLMOCK fcntl_mock(mmk_eq(int, 888), mmk_any(int), mmk_any(int))

TEST_CASE void
test_enable_nonblocking(struct test *t) {
    int ret;
    fcntl_mock_t fcntl_mock = mmk_mock("fcntl@self", fcntl_mock_t);

    /* When fcntl raise error. */
    ret = ERR;
    MMK_WHEN_RET(FCNTLMOCK, &ret, ENOENT);
    enable_nonblocking(888);
    MMKOK(FCNTLMOCK, 1);

    MMK_RESET(fcntl);
}

int
main() {
    struct test *t = TEST_BEGIN(LL_WARN);
    test_enable_nonblocking(t);
    return TEST_CLEAN(t);
}
