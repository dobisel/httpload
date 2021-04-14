#ifndef HTTPD_H
#define HTTPD_H

#include "ev.h"

struct httpd {
    struct evs;
};

void httpd_start(struct httpd *server);
#endif
