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
 * ev_common_peer_disconn()
 *********************************/

MMK_DEFINE(close_mock_t, int, int);

#define CLOSEMOCK close_mock(mmk_any(int))

static void
test_ev_common_peer_disconn() {
    int cret;
    close_mock_t close_mock = mmk_mock("close@self", close_mock_t);

    struct evs evs;
    struct peer *c = malloc(sizeof (struct peer));;
    c->fd = 888;

    /* When close successfull. */
    cret = OK;
    MMK_WHEN_RET(CLOSEMOCK, &cret, OK);
    ev_common_peer_disconn(&evs, c);
    MMKOK(CLOSEMOCK, 1);

    /* When close raised error. */
    cret = ERR;
    c = malloc(sizeof (struct peer));
    c->fd = 888;
    MMK_WHEN_RET(CLOSEMOCK, &cret, ENOENT);
    ev_common_peer_disconn(&evs, c);
    MMKOK(CLOSEMOCK, 2);

    /* When peer is null. */
    c = NULL;
    cret = OK;
    MMK_WHEN_RET(CLOSEMOCK, &cret, OK);
    ev_common_peer_disconn(&evs, c);
    MMKOK(CLOSEMOCK, 2);
    MMK_RESET(close);
}

/**********************************
 * ev_common_newconn()
 *********************************/

MMK_DEFINE(accept4_mock_t, int, int, struct sockaddr *, socklen_t *, int);

static void
newconn(struct evs *evs, struct peer *c) {
    EQI(c->fd, 888);
    EQI(c->status, PS_UNKNOWN);
    EQI(evs->listenfd, 777);
}

static void
newconn_close(struct evs *evs, struct peer *c) {
    EQI(c->fd, 888);
    EQI(c->status, PS_UNKNOWN);
    EQI(evs->listenfd, 777);
    c->status = PS_CLOSE;
}

#define ACCEPT4MOCK \
    accept4_mock(mmk_eq(int, 777), mmk_any(struct sockaddr *), \
            mmk_any(socklen_t *), mmk_eq(int, SOCK_NONBLOCK))

static void
test_ev_common_newconn() {
    int aret;
    int cret;
    accept4_mock_t accept4_mock = mmk_mock("accept4@self", accept4_mock_t);
    close_mock_t close_mock = mmk_mock("close@self", close_mock_t);

    struct evs evs = {
        .id = 1,
        .listenfd = 777,
        .on_connect = newconn,
    };
    struct peer *c;

    ev_common_init(&evs);

    /* When accept4 successfull. */
    aret = 888;
    MMK_WHEN_RET(ACCEPT4MOCK, &aret, OK);

    c = ev_common_newconn(&evs);
    EQI(c->fd, 888);
    EQI(c->status, PS_UNKNOWN);
    EQI(c->writerb.reader, 0);
    EQI(c->writerb.writer, 0);

    MMKOK(ACCEPT4MOCK, 1);

    /* When accept4 raised unhandled error. */
    aret = ERR;
    MMK_WHEN_RET(ACCEPT4MOCK, &aret, ENOENT);

    c = ev_common_newconn(&evs);
    ISNULL(c);

    MMKOK(ACCEPT4MOCK, 2);

    /* When accept4 raised EAGAIN. */
    aret = ERR;
    MMK_WHEN_RET(ACCEPT4MOCK, &aret, EAGAIN);

    c = ev_common_newconn(&evs);
    ISNULL(c);

    MMKOK(ACCEPT4MOCK, 3);

    /* When MAX_FDS reached. */
    aret = EV_MAXFDS;
    cret = 0;
    MMK_WHEN_RET(CLOSEMOCK, &cret, OK);
    MMK_WHEN_RET(ACCEPT4MOCK, &aret, OK);
    c = ev_common_newconn(&evs);
    ISNULL(c);
    MMKOK(CLOSEMOCK, 1);
    MMKOK(ACCEPT4MOCK, 4);

    /* When newconn callback request to reject connection. */
    aret = 888;
    cret = 0;
    MMK_WHEN_RET(CLOSEMOCK, &cret, OK);
    MMK_WHEN_RET(ACCEPT4MOCK, &aret, OK);
    evs.on_connect = newconn_close;
    c = ev_common_newconn(&evs);
    ISNULL(c);
    MMKOK(CLOSEMOCK, 2);
    MMKOK(ACCEPT4MOCK, 5);

    MMK_RESET(accept4);
    MMK_RESET(close);
    ev_common_peer_disconn(&evs, c);
}

/**********************************
 * ev_common_read
 *********************************/

MMK_DEFINE(read_mock_t, ssize_t, int, void *, size_t);

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

int
read_wrapper(int fd, void *buf, size_t count) {
    EQI(fd, 888);
    EQI(count, EV_READ_CHUNKSIZE);
    readstate.calls++;
    if (readstate.calls == 1) {
        return readstate.first;
    }
    if (readstate.calls == 2) {
        return readstate.second;
    }

    return readstate.third;
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
        .status = PS_UNKNOWN,
    };
    struct peer *c = &peer;

    ev_common_init(&ev);

    /* When read successfull. */
    MMK_WHEN_CALL(READMOCK, read_wrapper, OK);

    ev_common_read(&ev, c);
    EQI(c->status, PS_CLOSE);
    MMKOK(READMOCK, 3);

    /* When read raise EAGAIN. */
    c->status = PS_UNKNOWN;
    rret = ERR;
    MMK_WHEN_RET(READMOCK, &rret, EAGAIN);

    ev_common_read(&ev, c);
    EQI(c->status, PS_UNKNOWN);
    MMKOK(READMOCK, 4);

    /* When read buffer is full. */
    c->status = PS_UNKNOWN;
    rret = EV_READ_CHUNKSIZE;
    MMK_WHEN_RET(READMOCK, &rret, OK);

    ev_common_read(&ev, c);
    EQI(c->status, PS_CLOSE);
    MMKOK(READMOCK, 6);

    ev_common_deinit(&ev);
    MMK_RESET(read);
}

/**********************************
 * ev_common_write
 *********************************/

MMK_DEFINE(write_mock_t, ssize_t, int, const void *, size_t);

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
        .status = PS_UNKNOWN,
    };
    struct peer *c = &peer;

    rb_init(&c->writerb, c->writebuff, EV_WRITE_BUFFSIZE);
    EQI(rb_write(&c->writerb, "abcdefg", 7), RB_OK);

    /* When write successfull. */
    wret = OK;
    MMK_WHEN_RET(WRITEMOCK, &wret, OK);

    ev_common_write(&ev, c);
    EQI(c->status, PS_UNKNOWN);
    EQI(c->writerb.reader, 7);
    EQI(c->writerb.writer, 7);
    MMKOK(WRITEMOCK, 1);

    /* When write raised EAGAIN. */
    wret = ERR;
    MMK_WHEN_RET(WRITEMOCK, &wret, EAGAIN);
    EQI(rb_write(&c->writerb, "ijklmn", 6), RB_OK);
    ev_common_write(&ev, c);
    EQI(c->status, PS_WRITE);
    EQI(c->writerb.reader, 7);
    EQI(c->writerb.writer, 13);
    MMKOK(WRITEMOCK, 2);

    /* When write raised unhandled errors. */
    wret = ERR;
    MMK_WHEN_RET(WRITEMOCK, &wret, ENOENT);
    EQI(rb_write(&c->writerb, "opqrs", 5), RB_OK);
    ev_common_write(&ev, c);
    EQI(c->status, PS_CLOSE);
    EQI(c->writerb.reader, 7);
    EQI(c->writerb.writer, 18);
    MMKOK(WRITEMOCK, 3);
    MMK_RESET(write);
}

int
main() {
    static struct test test;

    log_setlevel(LL_ERROR);
    t = &test;
    SETUP(t);
    test_ev_common_write();
    test_ev_common_read();
    test_ev_common_newconn();
    test_ev_common_peer_disconn();
    return TEARDOWN(t);
}
