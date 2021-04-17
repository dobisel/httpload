#include "common.h"
#include "ringbuffer.h"
#include "logging.h"

#include <string.h>
#include <unistd.h>

#define MIN(x, y) (((x) > (y))? (y): (x))

int
rb_write(struct ringbuffer *b, const char *data, size_t len) {

    if (RB_AVAILABLE(b) < len) {
        return RB_ERR_INSUFFICIENT;
    }

    size_t toend = RB_FREE_TOEND(b);
    size_t chunklen = MIN(toend, len);

    memcpy(b->blob + b->writer, data, chunklen);
    b->writer = RB_WRITER_CALC(b, chunklen);
    b->writecounter += chunklen;

    if (len > chunklen) {
        len -= chunklen;
        memcpy(b->blob, data + chunklen, len);
        b->writer += len;
        b->writecounter += len;
    }
    return RB_OK;
}

size_t
rb_read(struct ringbuffer *b, char *data, size_t len) {
    size_t bytes;

    bytes = MIN(RB_USED_TOEND(b), len);
    if (bytes) {
        memcpy(data, b->blob + b->reader, bytes);
        b->reader = RB_READER_CALC(b, bytes);
        len -= bytes;
    }

    if (len) {
        len = MIN(RB_USED_TOEND(b), len);
        if (len) {
            memcpy(data + bytes, b->blob, len);
            b->reader += len;
            bytes += len;
        }
    }

    return bytes;
}

size_t
rb_dryread(struct ringbuffer *b, char *data, size_t len) {
    size_t toend = RB_USED_TOEND(b);
    size_t total = MIN(toend, len);

    memcpy(data, b->blob + b->reader, total);
    len -= total;

    if (len) {
        len = MIN(RB_USED(b) - total, len);
        memcpy(data + total, b->blob, len);
        total += len;
    }
    return total;
}

ssize_t
rb_readf(struct ringbuffer *b, int fd, size_t len) {
    size_t bytes;

    bytes = MIN(RB_USED_TOEND(b), len);
    if (bytes) {
        if (write(fd, b->blob + b->reader, bytes) < 0) {
            return ERR;
        }
        len -= bytes;
    }

    if (len) {
        len = MIN(RB_USED(b) - bytes, len);
        if (len) {
            if (write(fd, b->blob, len) < 0) {
                return ERR;
            }
            bytes += len;
        }
    }

    if (bytes) {
        RB_READER_SKIP(b, bytes);
    }
    return bytes;
}

int
rb_read_until(struct ringbuffer *b, char *data, size_t len,
              char *delimiter, size_t dlen, size_t *readlen) {
    size_t rl = rb_dryread(b, data, len);

    *readlen = rl;
    if (rl <= 0) {
        return RB_ERR_NOTFOUND;
    }
    char *f = memmem(data, rl, delimiter, dlen);

    if (f == NULL) {
        return RB_ERR_NOTFOUND;
    }
    *readlen = (f - data) + dlen;
    b->reader = RB_READER_CALC(b, *readlen);
    return RB_OK;
}

int
rb_dryread_until(struct ringbuffer *b, char *data, size_t len,
                 char *delimiter, size_t dlen, size_t *readlen) {
    size_t rl = rb_dryread(b, data, len);

    *readlen = rl;
    if (rl <= 0) {
        return RB_ERR_NOTFOUND;
    }
    char *f = memmem(data, rl, delimiter, dlen);

    if (f == NULL) {
        return RB_ERR_NOTFOUND;
    }
    *readlen = (f - data) + dlen;
    return RB_OK;
}

int
rb_read_until_chr(struct ringbuffer *b, char *data, size_t len,
                  char delimiter, size_t *readlen) {
    size_t rl = rb_dryread(b, data, len);

    *readlen = rl;
    if (rl <= 0) {
        return RB_ERR_NOTFOUND;
    }
    char *f = memchr(data, delimiter, rl);

    if (f == NULL) {
        return RB_ERR_NOTFOUND;
    }
    *readlen = (f - data) + 1;
    b->reader = RB_READER_CALC(b, *readlen);
    return RB_OK;
}

void
rb_init(struct ringbuffer *b, char *buff, size_t size) {
    b->size = size;
    b->reader = 0;
    b->writer = 0;
    b->writecounter = 0;
    b->blob = buff;
}
