#include "logging.h"
#include "fixtures/httpdmock.h"
#include "fixtures/curl.h"
#include "httpd.h"
#include "ev.h"

static int
httpdmock_sendrequest(struct httpdmock *m, const char *verb, const char *url, 
        const char *payload, size_t payloadsize) {
    m->out[0] = 0;
    m->err[0] = 0;
    return curl_request(verb, url, m->req_headers, m->optcb, m->out, m->err, 
            payload, payloadsize);
}

int
httpdmock_post(struct httpdmock *m, const char *url, const char *payload, 
        size_t len) {
    return httpdmock_sendrequest(m, "POST", url, payload, len);
}

int
httpdmock_get(struct httpdmock *m, const char *url) {
    return httpdmock_sendrequest(m, "GET", url, NULL, 0);
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
    if (httpd_start(&m->httpd) == ERR) {
        ERROR("Error starting httpd mock.");
        return;
    }
    
    sprintf(tmp, "http://localhost:%d", m->httpd.bind);
    m->url = strdup(tmp);
}

int 
httpdmock_stop(struct httpdmock *m) {
    int ret = httpd_stop(&m->httpd);
    free(m->url);
    return ret;
}
