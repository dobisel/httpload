#ifndef HTTPDMOCK_H
#define HTTPDMOCK_H

#include <sys/types.h>
#include <inttypes.h>


struct httpdmock {
    uint16_t port;
    uint8_t forks;
    pid_t *children;
};


int httpdmock_fork(struct httpdmock *);
int httpdmock_join(struct httpdmock *);

#endif
