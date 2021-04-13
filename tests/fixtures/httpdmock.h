#ifndef FIXTURES_HTTPDMOCK_H
#define FIXTURES_HTTPDMOCK_H

#include "fixtures/curl.h"
#include "httpd.h"

struct httpdmock {
    char out[CURL_BUFFMAX + 1];
    char err[CURL_BUFFMAX + 1];
    curlhook_t optcb;
    struct curl_slist *req_headers;
    struct httpd httpd;
    char *url;
};

int httpdmock_get(struct httpdmock *m);
void httpdmock_stop(struct httpdmock *m);
void httpdmock_start(struct httpdmock *m);


#endif
