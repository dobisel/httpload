#include "common.h"
#include "logging.h"
#include "fixtures/assert.h"
#include "ev_common.h"

#include <unistd.h>

/* third-party */
#include <mimick.h>

static struct test *t;

mmk_mock_define(write_mock_t, ssize_t, int, const void *, size_t);
mmk_mock_define(read_mock_t, ssize_t, int, void *, size_t);

static void
writefinishcb(struct ev *ev, struct peer *c) {
    EQI(c->fd, 888);
}

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

void
test_ev_common_read() {
    read_mock_t r = mmk_mock("read@self", read_mock_t);

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
    mmk_when(r(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t)),
             .then_call = (mmk_fn) read_wrapper,
             .then_return = &rret,
             .then_errno = OK
             );

    ev_common_read(&ev, c);
    EQI(c->state, PS_CLOSE);
    EQI(mmk_verify(r(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t)),
        .times = 3), 1);

    /* When read raise EAGAIN. */
    c->state = PS_UNKNOWN;
    rret = ERR;
    mmk_when(r(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t)),
             .then_return = &rret,
             .then_errno = EAGAIN 
             );

    ev_common_read(&ev, c);
    EQI(c->state, PS_UNKNOWN);
    EQI(mmk_verify(r(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t)),
        .times = 4), 1);

    /* When read buffer is full. */
    c->state = PS_UNKNOWN;
    rret = EV_READ_CHUNKSIZE;
    mmk_when(r(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t)),
             .then_return = &rret,
             .then_errno = OK
             );

    ev_common_read(&ev, c);
    EQI(c->state, PS_CLOSE);
    EQI(mmk_verify(r(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t)),
        .times = 6), 1);

    ev_common_deinit(&ev);
}

void
test_ev_common_write() {
    ssize_t wret;
    write_mock_t w = mmk_mock("write@self", write_mock_t);

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
    mmk_when(w(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t)),.
             then_return = &wret,.then_errno = OK);

    ev_common_write(&ev, c);
    EQI(c->state, PS_UNKNOWN);
    EQI(c->writerb.reader, 7);
    EQI(c->writerb.writer, 7);
    EQI(mmk_verify
        (w(mmk_eq(int, 888), mmk_any(void *), mmk_eq(size_t, 7)),.times =
         1), 1);

    /* When write raised EAGAIN. */
    wret = ERR;
    mmk_when(w(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t)),.
             then_return = &wret,.then_errno = EAGAIN);
    EQI(rb_write(&c->writerb, "ijklmn", 6), RB_OK);
    ev_common_write(&ev, c);
    EQI(c->state, PS_WRITE);
    EQI(c->writerb.reader, 7);
    EQI(c->writerb.writer, 13);
    EQI(mmk_verify
        (w(mmk_eq(int, 888), mmk_any(void *), mmk_eq(size_t, 6)),.times =
         1), 1);

    /* When write raised unhandled errors. */
    wret = ERR;
    mmk_when(w(mmk_eq(int, 888), mmk_any(void *), mmk_any(size_t)),.
             then_return = &wret,.then_errno = ENOENT);
    EQI(rb_write(&c->writerb, "opqrs", 5), RB_OK);
    ev_common_write(&ev, c);
    EQI(c->state, PS_CLOSE);
    EQI(c->writerb.reader, 7);
    EQI(c->writerb.writer, 18);
    EQI(mmk_verify
        (w(mmk_eq(int, 888), mmk_any(void *), mmk_eq(size_t, 11)),.times =
         1), 1);
    mmk_reset(write);
}

int
main() {
    static struct test test;
    t = &test;
    SETUP(t);
    test_ev_common_write();
    test_ev_common_read();
    return TEARDOWN(t);
}
