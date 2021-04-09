#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <inttypes.h>


struct httpd {
    uint16_t port;
    uint8_t forks;
    pid_t *children;
};


int httpd_fork(struct httpd *);
int httpd_join(struct httpd *);

#endif
