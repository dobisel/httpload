#ifndef HTTPD_H
#define HTTPD_H

#include "ev.h"

struct httpd {
    struct evs;
};

int httpd_start(struct httpd *server);
#endif
