#include "common.h"
#include "logging.h"
#include "fixtures/assert.h"
#include "ev_common.h"
#include "ev_epoll.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>

/* third-party */
#include <mimick.h>

static struct test *t;

#define EPFD    888

/* Mock close(2) */
MMK_DEFINE(close_mock_t, int, int);
#define CLOSE_MOCK close_mock(mmk_any(int))

/* Mock epoll_create1(2) */
MMK_DEFINE(epoll_create1_mock_t, int, int);
#define EPOLL_CREATE1_MOCK epoll_create1_mock(mmk_eq(int, EPOLL_CLOEXEC))

/* Mock epoll_ctl(2) */
MMK_DEFINE(epoll_ctl_mock_t, int, int, int, int, struct epoll_event *);
#define EPOLL_CTL_MOCK epoll_ctl_mock( \
        mmk_eq(int, EPFD), \
        mmk_any(int), \
        mmk_any(int), \
        mmk_any(struct epoll_event *))

/* Mock epoll_wait(2) */
MMK_DEFINE(epoll_wait_mock_t, int, int, struct epoll_event*, int, int);
#define EPOLL_WAIT_MOCK epoll_wait_mock( \
        mmk_eq(int, EPFD), \
        mmk_any(struct epoll_event *), \
        mmk_any(int), \
        mmk_any(int))

int 
epoll_wait_wrapper(int epfd, struct epoll_event *events, int maxevents, 
        int timeout) {
    EQI(epfd, EPFD);
    return 0;
}

void
test_ev_epoll_server_loop() {
    int close_ret;
    int epoll_create1_ret;
    int epoll_ctl_ret;
    struct evs evs;
    epoll_create1_mock_t epoll_create1_mock = mmk_mock("epoll_create1@self", 
            epoll_create1_mock_t);
    //epoll_ctl_mock_t epoll_ctl_mock = mmk_mock("epoll_ctl@self", 
    //        epoll_ctl_mock_t);
    //epoll_wait_mock_t epoll_wait_mock = mmk_mock("epoll_wait@self", 
    //        epoll_wait_mock_t);
    //close_mock_t close_mock = mmk_mock("close@self", close_mock_t);
    
    EQI(ev_epoll_server_init(&evs), OK);
    ISNOTNULL(evs.epoll);
    
    /* Simulate epoll_create1 error */
    epoll_create1_ret = ERR;
    MMK_WHEN_RET(EPOLL_CREATE1_MOCK, &epoll_create1_ret, ENOMEM);
    EQI(ev_epoll_server_loop(&evs), ERR);
    MMKOK(EPOLL_CREATE1_MOCK, 1);
    
    ///* Simulate new connection. */
    //evs.listenfd = 888;
    //close_ret = OK;
    //epoll_create1_ret = EPFD;
    //MMK_WHEN_RET(EPOLL_CREATE1_MOCK, &epoll_create1_ret, OK);
    //MMK_WHEN_RET(EPOLL_CTL_MOCK, &epoll_ctl_ret, OK);
    //MMK_WHEN_CALL(EPOLL_WAIT_MOCK, epoll_wait_wrapper, OK);
    //MMK_WHEN_RET(CLOSE_MOCK, &close_ret, OK);
    //
    ///* Run test */
    //EQI(ev_epoll_server_loop(&evs), OK);

    //ev_epoll_server_deinit(&evs);
    //MMKOK(CLOSE_MOCK, 1);
    //MMKOK(EPOLL_CREATE1_MOCK, 1);
    //MMKOK(EPOLL_CTL_MOCK, 1);
    //MMK_RESET(close);
    MMK_RESET(epoll_create1);
}

int
main() {
    static struct test test;

    log_setlevel(LL_WARN);
    t = &test;
    SETUP(t);
    test_ev_epoll_server_loop();
    return TEARDOWN(t);
}
