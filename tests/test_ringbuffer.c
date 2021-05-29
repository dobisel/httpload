/* local */
#include "common.h"
#include "ringbuffer.h"
#include "logging.h"
#include "fixtures/assert.h"

/* system */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* third-party */
#include <mimick.h>

#define S   8

TEST_CASE void
test_write_read(struct test *t) {
    char tmp[256];
    int tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    EQI(rb_write(&b, "abcdefgh", 8), RB_ERR_INSUFFICIENT);
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQNS(7, buff, "abcdefg");
    EQI(b.writer, 7);
    EQI(b.reader, 0);
    EQI(b.writecounter, 7);

    /* Write & Read */
    RB_RESET(&b);
    EQI(b.writecounter, 0);
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(b.writer, 7);
    tmplen += rb_read(&b, tmp + tmplen, 10);
    EQI(tmplen, 7);
    EQNS(7, tmp, "abcdefg");
    EQI(b.writer, 7);
    EQI(b.reader, 7);
    EQI(RB_AVAILABLE(&b), 7);
    EQI(b.writecounter, 7);

    EQI(rb_write(&b, "hi", 2), RB_OK);
    tmplen += rb_read(&b, tmp + tmplen, 10);
    EQI(tmplen, 9);
    EQI(b.writecounter, 9);

    EQI(rb_write(&b, "jklm", 4), RB_OK);
    tmplen += rb_read(&b, tmp + tmplen, 2);
    tmplen += rb_read(&b, tmp + tmplen, 2);
    EQI(tmplen, 13);
    EQNS(13, tmp, "abcdefghijklm");
    EQI(b.writecounter, 13);

    /* Read when no data available */
    EQI(rb_read(&b, tmp, 2), 0);
    EQI(b.writecounter, 13);
}

TEST_CASE void
test_dryread(struct test *t) {
    char tmp[256];
    int tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    /* Dry Read */
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    tmplen += rb_dryread(&b, tmp + tmplen, 2);
    EQI(tmplen, 2);
    EQNS(7, buff, "abcdefg");
    EQNS(2, tmp, "ab");
    EQI(b.writer, 7);
    EQI(b.reader, 0);
    EQI(b.writecounter, 7);

    RB_READER_SKIP(&b, 2);
    tmplen += rb_dryread(&b, tmp + tmplen, 10);
    EQI(tmplen, 7);
    EQNS(7, buff, "abcdefg");
    EQNS(7, tmp, "abcdefg");
    EQI(b.writer, 7);
    EQI(b.reader, 2);
    EQI(b.writecounter, 7);

    /* Skip */
    RB_READER_SKIP(&b, 4);
    EQI(b.reader, 6);
    EQI(RB_AVAILABLE(&b), 6);
    EQI(b.writecounter, 7);
}

TEST_CASE void
test_read_until(struct test *t) {
    char tmp[256];
    size_t tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(rb_read_until(&b, tmp, 7, "de", 2, &tmplen), RB_OK);
    EQI(tmplen, 5);
    EQNS(3, tmp, "abc");
    EQI(b.writer, 7);
    EQI(b.reader, 5);
    EQI(b.writecounter, 7);

    tmplen = 0;
    RB_RESET(&b);
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(rb_read_until(&b, tmp, 7, "ed", 2, &tmplen), RB_ERR_NOTFOUND);
    EQI(tmplen, 7);
    EQNS(7, tmp, "abcdefg");
    EQI(b.writer, 7);
    EQI(b.reader, 0);
    EQI(b.writecounter, 7);

    tmplen = 0;
    RB_RESET(&b);
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(rb_read_until(&b, tmp, 8, "ed", 2, &tmplen), RB_ERR_NOTFOUND);
    EQI(tmplen, 7);
    EQNS(7, tmp, "abcdefg");
    EQI(b.writer, 7);
    EQI(b.reader, 0);
    EQI(b.writecounter, 7);

    EQI(rb_read_until(&b, tmp, 8, "fg", 2, &tmplen), RB_OK);
    EQI(tmplen, 7);

    EQI(rb_read_until(&b, tmp, 8, "yz", 2, &tmplen), RB_ERR_NOTFOUND);
    EQI(tmplen, 0);
}

TEST_CASE void
test_read_until_chr(struct test *t) {
    char tmp[256];
    size_t tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(rb_read_until_chr(&b, tmp, 7, 'd', &tmplen), RB_OK);
    EQI(tmplen, 4);
    EQNS(4, tmp, "abcd");
    EQI(b.writer, 7);
    EQI(b.reader, 4);
    EQI(b.writecounter, 7);

    tmplen = 0;
    RB_RESET(&b);
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(rb_read_until_chr(&b, tmp, 8, 'z', &tmplen), RB_ERR_NOTFOUND);
    EQI(tmplen, 7);
    EQNS(7, tmp, "abcdefg");
    EQI(b.writer, 7);
    EQI(b.reader, 0);
    EQI(b.writecounter, 7);

    tmplen = 0;
    RB_RESET(&b);
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(rb_read_until_chr(&b, tmp, 4, 'z', &tmplen), RB_ERR_NOTFOUND);
    EQI(tmplen, 4);
    EQNS(4, tmp, "abcd");
    EQI(b.writer, 7);
    EQI(b.reader, 0);
    EQI(b.writecounter, 7);

    EQI(rb_read_until_chr(&b, tmp, 7, 'g', &tmplen), RB_OK);
    EQI(tmplen, 7);
    EQI(rb_read_until_chr(&b, tmp, 7, 'z', &tmplen), RB_ERR_NOTFOUND);
    EQI(tmplen, 0);
}

TEST_CASE void
test_dryread_until(struct test *t) {
    char tmp[256];
    size_t tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(rb_dryread_until(&b, tmp, 7, "de", 2, &tmplen), RB_OK);
    EQI(tmplen, 5);
    EQNS(3, tmp, "abc");
    EQI(b.writer, 7);
    EQI(b.reader, 0);
    EQI(b.writecounter, 7);

    tmplen = 0;
    RB_RESET(&b);
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(rb_dryread_until(&b, tmp, 7, "ed", 2, &tmplen), RB_ERR_NOTFOUND);
    EQI(tmplen, 7);
    EQNS(7, tmp, "abcdefg");
    EQI(b.writer, 7);
    EQI(b.reader, 0);
    EQI(b.writecounter, 7);

    tmplen = 0;
    RB_RESET(&b);
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);
    EQI(rb_dryread_until(&b, tmp, 8, "ed", 2, &tmplen), RB_ERR_NOTFOUND);
    EQI(tmplen, 7);
    EQNS(7, tmp, "abcdefg");
    EQI(b.writer, 7);
    EQI(b.reader, 0);
    EQI(b.writecounter, 7);

    EQI(rb_read_until(&b, tmp, 8, "fg", 2, &tmplen), RB_OK);
    EQI(tmplen, 7);
    EQI(rb_dryread_until(&b, tmp, 8, "yz", 2, &tmplen), RB_ERR_NOTFOUND);
    EQI(tmplen, 0);
}

TEST_CASE void
test_read_intofile(struct test *t) {
    char tmp[256];
    char buff[S];
    int p[2];
    struct ringbuffer b;

    pipe(p);

    rb_init(&b, buff, S);

    EQI(rb_write(&b, "abcdefg", 7), RB_OK);

    EQI(rb_readf(&b, p[1], 3), 3);
    EQI(b.reader, 3);
    EQI(rb_write(&b, "hij", 3), RB_OK);
    EQI(rb_readf(&b, p[1], 7), 7);

    EQI(read(p[0], tmp, 10), 10);
    EQNS(10, tmp, "abcdefghij");

    EQI(rb_readf(&b, p[1], 1), 0);
    close(p[1]);
    close(p[0]);
}

mmk_mock_define(write_mock_t, ssize_t, int, const void *, size_t);

TEST_CASE void
test_read_intofile_errors(struct test *t) {
    char buff[S];
    int p[2];
    struct ringbuffer b;
    ssize_t wret;
    write_mock_t w = mmk_mock("write", write_mock_t);

    pipe(p);
    rb_init(&b, buff, S);
    EQI(rb_write(&b, "abcdefg", 7), RB_OK);

    wret = -1;
    mmk_when(w
             (mmk_eq(int, p[1]), mmk_any(void *),
              mmk_any(size_t)),.then_return = &wret,.then_errno = EAGAIN);

    EQI(rb_readf(&b, p[1], 3), ERR);
    EQI(errno, EAGAIN);
    EQI(b.reader, 0);
    EQI(b.writer, 7);

    b.reader = 8;
    b.writer = 5;
    EQI(rb_readf(&b, p[1], 3), ERR);
    EQI(errno, EAGAIN);
    EQI(b.reader, 8);
    EQI(b.writer, 5);

    close(p[1]);
    close(p[0]);
    mmk_reset(w);
}

int
main() {
    struct test *t = TEST_BEGIN(LL_ERROR);
    test_write_read(t);
    test_dryread(t);
    test_dryread_until(t);
    test_read_until(t);
    test_read_until_chr(t);
    test_read_intofile(t);
    test_read_intofile_errors(t);
    return TEST_CLEAN(t);
}
