#include "common.h"
#include "logging.h"
#include "fixtures/assert.h"
#include "ev_common.h"

#include <sys/socket.h>
#include <unistd.h>

/* third-party */
#include <mimick.h>

static struct test *t;

/**********************************
 * Close
 *********************************/
mmk_mock_define(close_mock_t, int, int);

#define CLOSEMOCK close_mock(mmk_eq(int, 888))

static void
test_ev_common_peer_disconn() {
    int cret;
    close_mock_t close_mock = mmk_mock("close@self", close_mock_t);

    struct evs evs;
    struct peer *c = malloc(sizeof (struct peer));;
    c->fd = 888;

    /* When close successfull. */
    cret = OK;
    mmk_when(CLOSEMOCK,.then_return = &cret,.then_errno = OK);
    ev_common_peer_disconn(&evs, c);
    EQI(mmk_verify(CLOSEMOCK,.times = 1), 1);

    /* When close raised error. */
    cret = ERR;
    c = malloc(sizeof (struct peer));
    c->fd = 888;
    mmk_when(CLOSEMOCK,.then_return = &cret,.then_errno = ENOENT);
    ev_common_peer_disconn(&evs, c);
    EQI(mmk_verify(CLOSEMOCK,.times = 2), 1);

    /* When peer is null. */
    c = NULL;
    cret = OK;
    mmk_when(CLOSEMOCK,.then_return = &cret,.then_errno = OK);
    ev_common_peer_disconn(&evs, c);
    EQI(mmk_verify(CLOSEMOCK,.times = 2), 1);
}

/**********************************
 * Accept
 *********************************/
mmk_mock_define(accept4_mock_t, int, int, struct sockaddr *, socklen_t *,
                int);

static void
newconn(struct evs *evs, struct peer *c) {
    EQI(c->fd, 888);
    EQI(c->state, PS_UNKNOWN);
    EQI(evs->listenfd, 777);
}

#define ACCEPTMOCK \
    accept_mock(mmk_eq(int, 777), mmk_any(struct sockaddr *), \
            mmk_any(socklen_t *), mmk_eq(int, SOCK_NONBLOCK))

static void
test_ev_common_newconn() {
    int aret;
    accept4_mock_t accept_mock = mmk_mock("accept4@self", accept4_mock_t);

    struct evs evs = {
        .id = 1,
        .listenfd = 777,
        .on_connect = newconn,
    };
    struct peer *c;

    ev_common_init(&evs);

    /* When accept4 successfull. */
    aret = 888;
    mmk_when(ACCEPTMOCK,.then_return = &aret,.then_errno = OK);

    c = ev_common_newconn(&evs);
    EQI(c->fd, 888);
    EQI(c->state, PS_UNKNOWN);
    EQI(c->writerb.reader, 0);
    EQI(c->writerb.writer, 0);

    EQI(mmk_verify(ACCEPTMOCK,.times = 1), 1);

    /* When accept4 raised unhandled error. */
    aret = ERR;
    mmk_when(ACCEPTMOCK,.then_return = &aret,.then_errno = ENOENT);

    c = ev_common_newconn(&evs);
    ISNULL(c);

    EQI(mmk_verify(ACCEPTMOCK,.times = 2), 1);

    /* When accept4 raised EAGAIN. */
    aret = ERR;
    mmk_when(ACCEPTMOCK,.then_return = &aret,.then_errno = EAGAIN);

    c = ev_common_newconn(&evs);
    ISNULL(c);

    EQI(mmk_verify(ACCEPTMOCK,.times = 3), 1);

    /* When MAX_FDS reached. */
    aret = EV_MAXFDS;
    mmk_when(ACCEPTMOCK,.then_return = &aret,.then_errno = OK);

    c = ev_common_newconn(&evs);
    ISNULL(c);
    EQI(mmk_verify(ACCEPTMOCK,.times = 4), 1);

    ev_common_peer_disconn(&evs, c);
}

/**********************************
 * Read
 *********************************/

mmk_mock_define(read_mock_t, ssize_t, int, void *, size_t);

static void
recvd(struct ev *ev, struct peer *c, const char *data, size_t len) {
    EQI(c->fd, 888);
    EQI(len, 0);
    EQNS(len, data, "abcdefg");
}

static struct {
    ssize_t first;
    ssize_t second;
    ssize_t third;
    int calls;
} readstate = {
    .first = 5,
    .second = 3,
    .third = 0,
    .calls = 0,
};

static ssize_t rret;

void
read_wrapper(int fd, void *buf, size_t count) {
    EQI(fd, 888);
    EQI(count, EV_READ_CHUNKSIZE);
    readstate.calls++;
    if (readstate.calls == 1) {
        rret = readstate.first;
    }
    else if (readstate.calls == 2) {
        rret = readstate.second;
    }
    else if (readstate.calls == 3) {
        rret = readstate.third;
    }
}

#define READMOCK read_mock(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t))

void
test_ev_common_read() {
    read_mock_t read_mock = mmk_mock("read@self", read_mock_t);

    struct ev ev = {
        .on_recvd = recvd
    };
    struct peer peer = {
        .fd = 888,
        .state = PS_UNKNOWN,
    };
    struct peer *c = &peer;

    ev_common_init(&ev);

    /* When read successfull. */
    mmk_when(READMOCK,.then_call =
             (mmk_fn) read_wrapper,.then_return = &rret,.then_errno = OK);

    ev_common_read(&ev, c);
    EQI(c->state, PS_CLOSE);
    EQI(mmk_verify(READMOCK,.times = 3), 1);

    /* When read raise EAGAIN. */
    c->state = PS_UNKNOWN;
    rret = ERR;
    mmk_when(READMOCK,.then_return = &rret,.then_errno = EAGAIN);

    ev_common_read(&ev, c);
    EQI(c->state, PS_UNKNOWN);
    EQI(mmk_verify(READMOCK,.times = 4), 1);

    /* When read buffer is full. */
    c->state = PS_UNKNOWN;
    rret = EV_READ_CHUNKSIZE;
    mmk_when(READMOCK,.then_return = &rret,.then_errno = OK);

    ev_common_read(&ev, c);
    EQI(c->state, PS_CLOSE);
    EQI(mmk_verify(READMOCK,.times = 6), 1);

    ev_common_deinit(&ev);
}

/**********************************
 * Write
 *********************************/

mmk_mock_define(write_mock_t, ssize_t, int, const void *, size_t);

static void
writefinishcb(struct ev *ev, struct peer *c) {
    EQI(c->fd, 888);
}

#define WRITEMOCK \
    write_mock(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t))

void
test_ev_common_write() {
    ssize_t wret;
    write_mock_t write_mock = mmk_mock("write@self", write_mock_t);

    struct ev ev = {
        .on_writefinish = writefinishcb,
    };
    struct peer peer = {
        .fd = 888,
        .state = PS_UNKNOWN,
    };
    struct peer *c = &peer;

    rb_init(&c->writerb, c->writebuff, EV_WRITE_BUFFSIZE);
    EQI(rb_write(&c->writerb, "abcdefg", 7), RB_OK);

    /* When write successfull. */
    wret = OK;
    mmk_when(WRITEMOCK,.then_return = &wret,.then_errno = OK);

    ev_common_write(&ev, c);
    EQI(c->state, PS_UNKNOWN);
    EQI(c->writerb.reader, 7);
    EQI(c->writerb.writer, 7);
    EQI(mmk_verify(WRITEMOCK,.times = 1), 1);

    /* When write raised EAGAIN. */
    wret = ERR;
    mmk_when(WRITEMOCK,.then_return = &wret,.then_errno = EAGAIN);
    EQI(rb_write(&c->writerb, "ijklmn", 6), RB_OK);
    ev_common_write(&ev, c);
    EQI(c->state, PS_WRITE);
    EQI(c->writerb.reader, 7);
    EQI(c->writerb.writer, 13);
    EQI(mmk_verify(WRITEMOCK,.times = 2), 1);

    /* When write raised unhandled errors. */
    wret = ERR;
    mmk_when(WRITEMOCK,.then_return = &wret,.then_errno = ENOENT);
    EQI(rb_write(&c->writerb, "opqrs", 5), RB_OK);
    ev_common_write(&ev, c);
    EQI(c->state, PS_CLOSE);
    EQI(c->writerb.reader, 7);
    EQI(c->writerb.writer, 18);
    EQI(mmk_verify(WRITEMOCK,.times = 3), 1);
    mmk_reset(write);
}

int
main() {
    static struct test test;

    t = &test;
    SETUP(t);
    test_ev_common_write();
    test_ev_common_read();
    test_ev_common_newconn();
    test_ev_common_peer_disconn();
    return TEARDOWN(t);
}
