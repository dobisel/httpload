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
void httpd_terminate(struct httpd *m);
int httpd_join(struct httpd *);

#endif
