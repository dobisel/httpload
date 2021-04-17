#include "logging.h"
#include "fixtures/httpdmock.h"
#include "fixtures/curl.h"
#include "httpd.h"

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
    
    httpd_start(&m->httpd);
    
    sprintf(tmp, "http://localhost:%d", m->httpd.bind);
    m->url = strdup(tmp);
}

void 
httpdmock_stop(struct httpdmock *m) {
    ev_common_terminate((struct ev*)&m->httpd);
    free(m->url);
    httpd_join(&m->httpd);
}
