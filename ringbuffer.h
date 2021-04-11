#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdlib.h>

#define RB_OK                    0
#define RB_ERR_INSUFFICIENT     -100
#define RB_ERR_NOTFOUND         -101


#define RB_CALC(b, n)         ((n) & ((b)->size - 1))
#define RB_USED(b)            RB_CALC(b, (b)->writer - (b)->reader)
#define RB_AVAILABLE(b)       RB_CALC(b, (b)->reader - ((b)->writer + 1))
#define RB_WRITER_CALC(b, n)  RB_CALC(b, (b)->writer + (n))
#define RB_READER_CALC(b, n)  RB_CALC(b, (b)->reader + (n))
#define RB_READER_SKIP(b, n)  (b)->reader = RB_READER_CALC((b), (n))
#define RB_RESET(b) ({ \
    (b)->writecounter = 0; \
    (b)->reader = 0; \
    (b)->writer = 0; \
})


#define RB_USED_TOEND(b) \
	({ssize_t end = ((b)->size) - ((b)->reader); \
	  ssize_t n = (((b)->writer) + end) & (((b)->size)-1); \
	  n < end ? n : end;})

#define RB_FREE_TOEND(b) \
	({ssize_t end = ((b)->size) - 1 - ((b)->writer); \
	  ssize_t n = (end + ((b)->reader)) & (((b)->size)-1); \
	  n <= end ? n : end+1;})



struct ringbuffer{
    size_t size;
    int reader;
    int writer;
    size_t writecounter;
    char *blob;
};


size_t rb_read(struct ringbuffer *b, char *data, size_t len);
size_t rb_dryread(struct ringbuffer *b, char *data, size_t len);
int rb_pushone(struct ringbuffer *rb, char byte);
int rb_write(struct ringbuffer *b, const char *data, size_t len);
void rb_init(struct ringbuffer *b, char *buff, size_t size);
int rb_read_until_chr(struct ringbuffer *b, char *data, size_t len,
        char delimiter, size_t *readlen);
int rb_read_until(struct ringbuffer *b, char *data, size_t len,
        char *delimiter, size_t dlen, size_t *readlen);
int rb_dryread_until(struct ringbuffer *b, char *data, size_t len,
        char *delimiter, size_t dlen, size_t *readlen);

ssize_t rb_readf(struct ringbuffer *b, int fd, size_t len);

#endif

