#include "logging.h"
#include "fixtures/httpdmock.h"
#include "fixtures/curl.h"
#include "httpd.h"
#include "ev.h"

int
httpdmock_get(struct httpdmock *m) {
    m->out[0] = 0;
    m->err[0] = 0;
    return curl_get(m->url, m->req_headers, m->optcb, m->out, m->err);
}

void 
httpdmock_start(struct httpdmock *m) {
    char tmp[128];

    m->httpd.forks = 1;
    m->httpd.bind = 0;
    m->optcb = NULL;
    m->req_headers = NULL;
    fflush(stdout);    
    fflush(stderr);    
    httpd_start(&m->httpd);
    
    sprintf(tmp, "http://localhost:%d", m->httpd.bind);
    m->url = strdup(tmp);
}

int 
httpdmock_stop(struct httpdmock *m) {
    int ret = httpd_stop(&m->httpd);
    free(m->url);
    return ret;
}
