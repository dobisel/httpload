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
#define LISTENFD    777
#define NOERROR 0

/* Mock close(2) */
MMK_DEFINE(close_mock_t, int, int);
#define CLOSE_MOCK close_mock(mmk_any(int))

/* Mock epoll_create1(2) */
MMK_DEFINE(epoll_create1_mock_t, int, int);
#define EPOLL_CREATE1_MOCK epoll_create1_mock(mmk_any(int))

/* Mock epoll_ctl(2) */
MMK_DEFINE(epoll_ctl_mock_t, int, int, int, int, struct epoll_event *);
#define EPOLL_CTL_MOCK epoll_ctl_mock( \
        mmk_any(int), \
        mmk_any(int), \
        mmk_any(int), \
        mmk_any(struct epoll_event *))

/* Mock epoll_wait(2) */
MMK_DEFINE(epoll_wait_mock_t, int, int, struct epoll_event*, int, int);
#define EPOLL_WAIT_MOCK epoll_wait_mock( \
        mmk_any(int), \
        mmk_any(struct epoll_event *), \
        mmk_any(int), \
        mmk_any(int))

/* Mock accept4(2) */
MMK_DEFINE(accept4_mock_t, int, int, struct sockaddr *, socklen_t *, int);
#define ACCEPT4_MOCK accept4_mock( \
        mmk_any(int), \
        mmk_any(struct sockaddr *), \
        mmk_any(socklen_t *), \
        mmk_any(int))

static bool epctl_firsttime = true;

int 
epoll_ctl_wrapper(int epfd, int op, int fd, struct epoll_event *event) {
    EQI(epfd, EPFD);
    if (fd == LISTENFD && op == EPOLL_CTL_MOD) {
        if (epctl_firsttime) {
            return ERR;
        }
        return OK;
    }
    return OK;
}

static struct epoll_event epwait_events[2];

int 
epoll_wait_wrapper(int epfd, struct epoll_event *events, int maxevents, 
        int timeout) {
    EQI(epfd, EPFD);
    events[0] = epwait_events[0];
    events[1] = epwait_events[1];
    return 2;
}

void
test_ev_epoll_server_loop() {
    //int close_ret;
    int epoll_create1_ret;
    int epoll_wait_ret;
    int accept4_ret;
    struct evs evs = {
        .listenfd = 777
    };
    epoll_create1_mock_t epoll_create1_mock = mmk_mock(
            "epoll_create1@self", 
            epoll_create1_mock_t);
    epoll_ctl_mock_t epoll_ctl_mock = mmk_mock(
            "epoll_ctl@self", 
            epoll_ctl_mock_t);
    epoll_wait_mock_t epoll_wait_mock = mmk_mock(
            "epoll_wait@self", 
            epoll_wait_mock_t);
    accept4_mock_t accept4_mock = mmk_mock(
            "accept4@self", 
            accept4_mock_t);
    //close_mock_t close_mock = mmk_mock("close@self", close_mock_t);
    
    EQI(ev_epoll_server_init(&evs), OK);
    ISNOTNULL(evs.epoll);
    
    epoll_create1_ret = ERR;
    epoll_wait_ret = ERR;
    MMK_WHEN_RET(EPOLL_CREATE1_MOCK, &epoll_create1_ret, ENOMEM);
    MMK_WHEN_CALL(EPOLL_CTL_MOCK, epoll_ctl_wrapper, NOERROR);
    MMK_WHEN_RET(EPOLL_WAIT_MOCK, &epoll_wait_ret, ENOENT);

    /* Simulate epoll_create1 error */
    EQI(ev_epoll_server_loop(&evs), ERR);
    MMKOK(EPOLL_CREATE1_MOCK, 1);

    /* Simulate when epoll_wait returns error. */
    epoll_create1_ret = EPFD;
    EQI(ev_epoll_server_loop(&evs), ERR);

    MMKOK(EPOLL_CREATE1_MOCK, 2);
    MMKOK(EPOLL_CTL_MOCK, 1);
    MMKOK(EPOLL_WAIT_MOCK, 1);
 
    /* Simulate EPOLLERR. */
    epwait_events[0].events = EPOLLERR;
    MMK_WHEN_CALL(EPOLL_WAIT_MOCK, epoll_wait_wrapper, NOERROR);
    EQI(ev_epoll_server_loop(&evs), ERR);
    MMKOK(EPOLL_CTL_MOCK, 2);
    MMKOK(EPOLL_WAIT_MOCK, 2);
    
    /* Simulate error when register listen fd after new connection. */
    epwait_events[0].data.fd = LISTENFD;
    epwait_events[0].events = EPOLLIN;
    MMK_WHEN_CALL(EPOLL_WAIT_MOCK, epoll_wait_wrapper, NOERROR);
    EQI(ev_epoll_server_loop(&evs), ERR);
    MMKOK(EPOLL_CTL_MOCK, 4);
    MMKOK(EPOLL_WAIT_MOCK, 3);

    /* Simulate error when accepting a new connection. */
    epctl_firsttime = false;
    epwait_events[0].data.fd = LISTENFD;
    epwait_events[0].events = EPOLLIN;
    epwait_events[1].events = EPOLLERR;
    accept4_ret = ERR;
    MMK_WHEN_RET(ACCEPT4_MOCK, &accept4_ret, ENOENT);
    EQI(ev_epoll_server_loop(&evs), ERR);
    MMKOK(EPOLL_CTL_MOCK, 6);
    MMKOK(EPOLL_WAIT_MOCK, 4);
    MMKOK(ACCEPT4_MOCK, 1);
    
    /* Simulate EPOLLRDHUP */

    //close_ret = OK;
    //epoll_create1_ret = EPFD;
    //MMK_WHEN_RET(EPOLL_CREATE1_MOCK, &epoll_create1_ret, OK);
    //MMK_WHEN_RET(EPOLL_CTL_MOCK, &epoll_ctl_ret, OK);
    //MMK_WHEN_CALL(EPOLL_WAIT_MOCK, epoll_wait_wrapper, OK);
    //MMK_WHEN_RET(CLOSE_MOCK, &close_ret, OK);
    //
    ///* Run test */
    //EQI(ev_epoll_server_loop(&evs), OK);

    ev_epoll_server_deinit(&evs);
    //MMKOK(CLOSE_MOCK, 1);
    //MMKOK(EPOLL_CREATE1_MOCK, 1);
    //MMKOK(EPOLL_CTL_MOCK, 1);
    //MMK_RESET(close);
    MMK_RESET(epoll_create1);
    MMK_RESET(epoll_ctl);
    MMK_RESET(epoll_wait);
}

int
main() {
    static struct test test;

    log_setlevel(LL_DEBUG);
    t = &test;
    SETUP(t);
    test_ev_epoll_server_loop();
    return TEARDOWN(t);
}
