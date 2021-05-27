#ifndef OPTIONS_H
#define OPTIONS_H

// Must be power of 2
#define EV_WRITE_BUFFSIZE          1024 * 32    // 2 ** 15
#define EV_READ_BUFFSIZE           1024 * 64    // 2 ** 16

#define TCP_BACKLOG           16

#define EV_MAXFDS                   1024
#define EV_BATCHSIZE                128

#define EV_WRITE_CHUNKSIZE         1024 * 16
#define EV_READ_CHUNKSIZE          1024 * 32
#endif
