#include "common.h"
#include "logging.h"
#include "fixtures/assert.h"
#include <stdio.h>
#include <http_parser.h>

static char url[100];
static char body[100];
static size_t bodylen = 0;

int
url_cb(http_parser * p, const char *at, size_t len) {
    memcpy(url, at, len);
    url[len] = 0;
    return 0;
}

int
body_cb(http_parser * p, const char *at, size_t len) {
    memcpy(body + bodylen, at, len);
    bodylen += len;
    body[bodylen] = 0;
    return 0;
}

static struct test test;
static http_parser p;

static http_parser_settings settings = {
    .on_url = url_cb,
    .on_body = body_cb,
};

size_t
req_chunk(const char *b) {
    size_t l;

    if (b == NULL) {
        l = 0;
    }
    else {
        l = strlen(b);
    }
    return http_parser_execute(&p, &settings, b, l);
}

int
req(const char *b) {
    url[0] = body[0] = 0;
    bodylen = 0;
    size_t rl = req_chunk(b);
    struct test *t = &test;

    EQI(p.http_errno, 0);
    req_chunk(NULL);
    EQI(p.http_errno, 0);
    return rl;
}

void
test_httpparser_get(struct test *t) {
    http_parser_init(&p, HTTP_REQUEST);
    req("GET / HTTP/1.1" RN "Content-Length: 5" RN RN "12345");
    EQS(body, "12345");

    req("GET /" RN RN);
    EQS(body, "");

    bodylen = 0;
    req_chunk("GET / HT");
    req_chunk("TP/1.1" RN "Conten");
    req_chunk("t-Length: 5" RN RN "123");
    req_chunk("45");
    req_chunk(NULL);
    EQS(body, "12345");
}

int
main() {
    log_setlevel(LL_DEBUG);
    SETUP(&test);
    test_httpparser_get(&test);
    return TEARDOWN(&test);
}
