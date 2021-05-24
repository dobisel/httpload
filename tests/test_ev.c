#include "common.h"
#include "logging.h"
#include "helpers.h"
#include "inttypes.h"
#include "ev_common.h"
#include "ev_epoll.h"
#include "ev.h"
#include "fixtures/assert.h"

#include <unistd.h>

/* third-party */
#include <mimick.h>

static struct test *t;

/**********************************
 * ev_server_start()
 *********************************/

MMK_DEFINE(malloc_mock_t, void *, size_t);
MMK_DEFINE(calloc_mock_t, void *, size_t, size_t);

#define MALLOC_M    malloc_mock(mmk_any(size_t))
#define CALLOC_M    calloc_mock(mmk_any(size_t), mmk_any(size_t))

static void
test_ev_server_start() {
    struct evs evs = {
        .forks = 1,
    };
    struct ev_epoll *malloc_ret = NULL;
    struct ev_epoll *calloc_ret = NULL;
    malloc_mock_t malloc_mock = mmk_mock("malloc@self", malloc_mock_t);
    calloc_mock_t calloc_mock = mmk_mock("calloc@self", calloc_mock_t);

    MMK_WHEN_RET(MALLOC_M, &malloc_ret, ENOMEM);
    EQI(ev_server_start(&evs), ERR);
    MMKOK(MALLOC_M, 1);
    close(evs.listenfd);
    MMK_RESET(malloc);

    calloc_ret = NULL;
    MMK_WHEN_RET(CALLOC_M, &calloc_ret, ENOMEM);
    EQI(ev_server_start(&evs), ERR);
    MMKOK(CALLOC_M, 1);

    MMK_RESET(calloc);
}

int
main() {
    static struct test test;

    log_setlevel(LL_ERROR);
    t = &test;
    SETUP(t);
    test_ev_server_start();
    return TEARDOWN(t);
}
