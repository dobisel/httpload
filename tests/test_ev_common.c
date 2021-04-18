#include "common.h"
#include "logging.h"
#include "fixtures/assert.h"
#include "ev_common.h"

/* third-party */
#include <mimick.h>

mmk_mock_define(write_mock_t, ssize_t, int, const void *, size_t);

void
writefinishcb_keepconn(struct ev *ev, struct peer *c) {
    struct test *t = (struct test *) c->handler;

    EQI(c->fd, 888);
}

void
test_ev_common_write(struct test *t) {
    ssize_t wret;
    write_mock_t w = mmk_mock("write@self", write_mock_t);

    struct ev ev = {
        .on_writefinish = writefinishcb_keepconn,
    };
    struct peer peer = {
        .fd = 888,
        .state = PS_UNKNOWN,
        .handler = t,
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
}

int
main() {
    struct test t;

    SETUP(&t);
    test_ev_common_write(&t);
    return TEARDOWN(&t);
}
