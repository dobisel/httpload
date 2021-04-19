#ifndef HTTPD_H
#define HTTPD_H

#include "ev_epoll.h"

struct httpd {
    struct evs;
};

int httpd_stop(struct httpd *server);
void httpd_start(struct httpd *server);
void httpd_terminate(struct httpd *server);
int httpd_join(struct httpd *server);

#endif
