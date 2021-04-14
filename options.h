#ifndef OPTIONS_H
#define OPTIONS_H

// Must be power of 2
#define EV_WRITEBUFF_SIZE          1024 * 64    // 2 ** 16

#define EV_WRITE_CHUNKSIZE         1024 * 16

#define TCP_BACKLOG           16

#define EV_MAXFDS                   32

#define EV_READSIZE           1024 * 64
#endif
