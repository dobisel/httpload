#ifndef HTTPD_H
#define HTTPD_H

#include "ev_common.h"

/* Third party */
#include <http_parser.h>

struct httpd {
    struct evs;
    uint32_t max_headers_size;
};

int httpd_stop(struct httpd *server);
int httpd_start(struct httpd *server);
void httpd_terminate(struct httpd *server);
int httpd_join(struct httpd *server);

#endif
