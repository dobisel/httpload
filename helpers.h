#ifndef HELPERS_H
#define HELPERS_H

#include <inttypes.h>
#include <netinet/in.h>

int
    enable_nonblocking(int fd);

int
    tcp_listen(uint16_t * port);

#endif
