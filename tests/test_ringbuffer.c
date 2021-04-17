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

void
test_write_read() {
    char tmp[256];
    int tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    eqint(RB_ERR_INSUFFICIENT, rb_write(&b, "abcdefgh", 8));
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqnstr("abcdefg", buff, 7);
    eqint(7, b.writer);
    eqint(0, b.reader);
    eqint(7, b.writecounter);

    /* Write & Read */
    RB_RESET(&b);
    eqint(0, b.writecounter);
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(7, b.writer);
    tmplen += rb_read(&b, tmp + tmplen, 10);
    eqint(7, tmplen);
    eqnstr("abcdefg", tmp, 7);
    eqint(7, b.writer);
    eqint(7, b.reader);
    eqint(7, RB_AVAILABLE(&b));
    eqint(7, b.writecounter);

    eqint(RB_OK, rb_write(&b, "hi", 2));
    tmplen += rb_read(&b, tmp + tmplen, 10);
    eqint(9, tmplen);
    eqint(9, b.writecounter);

    eqint(RB_OK, rb_write(&b, "jklm", 4));
    tmplen += rb_read(&b, tmp + tmplen, 2);
    tmplen += rb_read(&b, tmp + tmplen, 2);
    eqint(13, tmplen);
    eqnstr("abcdefghijklm", tmp, 13);
    eqint(13, b.writecounter);

    /* Read when no data available */
    eqint(0, rb_read(&b, tmp, 2));
    eqint(13, b.writecounter);
}

void
test_dryread() {
    char tmp[256];
    int tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    /* Dry Read */
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    tmplen += rb_dryread(&b, tmp + tmplen, 2);
    eqint(2, tmplen);
    eqnstr("abcdefg", buff, 7);
    eqnstr("ab", tmp, 2);
    eqint(7, b.writer);
    eqint(0, b.reader);
    eqint(7, b.writecounter);

    RB_READER_SKIP(&b, 2);
    tmplen += rb_dryread(&b, tmp + tmplen, 10);
    eqint(7, tmplen);
    eqnstr("abcdefg", buff, 7);
    eqnstr("abcdefg", tmp, 7);
    eqint(7, b.writer);
    eqint(2, b.reader);
    eqint(7, b.writecounter);

    /* Skip */
    RB_READER_SKIP(&b, 4);
    eqint(6, b.reader);
    eqint(6, RB_AVAILABLE(&b));
    eqint(7, b.writecounter);
}

void
test_read_until() {
    char tmp[256];
    size_t tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(RB_OK, rb_read_until(&b, tmp, 7, "de", 2, &tmplen));
    eqint(5, tmplen);
    eqnstr("abc", tmp, 3);
    eqint(7, b.writer);
    eqint(5, b.reader);
    eqint(7, b.writecounter);

    tmplen = 0;
    RB_RESET(&b);
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(RB_ERR_NOTFOUND, rb_read_until(&b, tmp, 7, "ed", 2, &tmplen));
    eqint(7, tmplen);
    eqnstr("abcdefg", tmp, 7);
    eqint(7, b.writer);
    eqint(0, b.reader);
    eqint(7, b.writecounter);

    tmplen = 0;
    RB_RESET(&b);
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(RB_ERR_NOTFOUND, rb_read_until(&b, tmp, 8, "ed", 2, &tmplen));
    eqint(7, tmplen);
    eqnstr("abcdefg", tmp, 7);
    eqint(7, b.writer);
    eqint(0, b.reader);
    eqint(7, b.writecounter);

    eqint(RB_OK, rb_read_until(&b, tmp, 8, "fg", 2, &tmplen));
    eqint(7, tmplen);

    eqint(RB_ERR_NOTFOUND, rb_read_until(&b, tmp, 8, "yz", 2, &tmplen));
    eqint(0, tmplen);
}

void
test_read_until_chr() {
    char tmp[256];
    size_t tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(RB_OK, rb_read_until_chr(&b, tmp, 7, 'd', &tmplen));
    eqint(4, tmplen);
    eqnstr("abcd", tmp, 4);
    eqint(7, b.writer);
    eqint(4, b.reader);
    eqint(7, b.writecounter);

    tmplen = 0;
    RB_RESET(&b);
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(RB_ERR_NOTFOUND, rb_read_until_chr(&b, tmp, 8, 'z', &tmplen));
    eqint(7, tmplen);
    eqnstr("abcdefg", tmp, 7);
    eqint(7, b.writer);
    eqint(0, b.reader);
    eqint(7, b.writecounter);

    tmplen = 0;
    RB_RESET(&b);
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(RB_ERR_NOTFOUND, rb_read_until_chr(&b, tmp, 4, 'z', &tmplen));
    eqint(4, tmplen);
    eqnstr("abcd", tmp, 4);
    eqint(7, b.writer);
    eqint(0, b.reader);
    eqint(7, b.writecounter);

    eqint(RB_OK, rb_read_until_chr(&b, tmp, 7, 'g', &tmplen));
    eqint(7, tmplen);
    eqint(RB_ERR_NOTFOUND, rb_read_until_chr(&b, tmp, 7, 'z', &tmplen));
    eqint(0, tmplen);
}

void
test_dryread_until() {
    char tmp[256];
    size_t tmplen = 0;
    char buff[S];
    struct ringbuffer b;

    rb_init(&b, buff, S);

    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(RB_OK, rb_dryread_until(&b, tmp, 7, "de", 2, &tmplen));
    eqint(5, tmplen);
    eqnstr("abc", tmp, 3);
    eqint(7, b.writer);
    eqint(0, b.reader);
    eqint(7, b.writecounter);

    tmplen = 0;
    RB_RESET(&b);
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(RB_ERR_NOTFOUND, rb_dryread_until(&b, tmp, 7, "ed", 2, &tmplen));
    eqint(7, tmplen);
    eqnstr("abcdefg", tmp, 7);
    eqint(7, b.writer);
    eqint(0, b.reader);
    eqint(7, b.writecounter);

    tmplen = 0;
    RB_RESET(&b);
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));
    eqint(RB_ERR_NOTFOUND, rb_dryread_until(&b, tmp, 8, "ed", 2, &tmplen));
    eqint(7, tmplen);
    eqnstr("abcdefg", tmp, 7);
    eqint(7, b.writer);
    eqint(0, b.reader);
    eqint(7, b.writecounter);

    eqint(RB_OK, rb_read_until(&b, tmp, 8, "fg", 2, &tmplen));
    eqint(7, tmplen);
    eqint(RB_ERR_NOTFOUND, rb_dryread_until(&b, tmp, 8, "yz", 2, &tmplen));
    eqint(0, tmplen);
}

void
test_read_intofile() {
    char tmp[256];
    char buff[S];
    int p[2];
    struct ringbuffer b;

    pipe(p);

    rb_init(&b, buff, S);

    eqint(RB_OK, rb_write(&b, "abcdefg", 7));

    eqint(3, rb_readf(&b, p[1], 3));
    eqint(3, b.reader);
    eqint(RB_OK, rb_write(&b, "hij", 3));
    eqint(7, rb_readf(&b, p[1], 7));

    eqint(10, read(p[0], tmp, 10));
    eqnstr("abcdefghij", tmp, 10);

    eqint(0, rb_readf(&b, p[1], 1));
    close(p[1]);
    close(p[0]);
}

mmk_mock_define (write_mock_t, ssize_t, int, const void *, size_t);

void
test_read_intofile_errors() {
    char buff[S];
    int p[2];
    struct ringbuffer b;
    ssize_t wret;
    write_mock_t w = mmk_mock("write", write_mock_t);

    pipe(p);
    rb_init(&b, buff, S);
    eqint(RB_OK, rb_write(&b, "abcdefg", 7));

    wret = -1;
    mmk_when(w(mmk_eq(int, p[1]), mmk_any(void *), mmk_any(size_t)),
        .then_return = &wret,
        .then_errno = EAGAIN);
    
    eqint(ERR, rb_readf(&b, p[1], 3));
    eqint(EAGAIN, errno);
    eqint(0, b.reader);
    eqint(7, b.writer);
  
    b.reader = 8;
    b.writer = 5;
    eqint(ERR, rb_readf(&b, p[1], 3));
    eqint(EAGAIN, errno);
    eqint(8, b.reader);
    eqint(5, b.writer);
    
    close(p[1]);
    close(p[0]);
    mmk_reset(w);
}

int
main() {
    log_setlevel(LL_DEBUG);
    test_write_read();
    test_dryread();
    test_dryread_until();
    test_read_until();
    test_read_until_chr();
    test_read_intofile();
    test_read_intofile_errors();
}
