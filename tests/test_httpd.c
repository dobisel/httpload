#include "logging.h"
#include "fixtures/assert.h"
#include "fixtures/httpdmock.h"
#include "fixtures/stdcapt.h"

#include <stdlib.h>

TEST_CASE void
test_single_packet(struct test *t) {
    struct httpdmock m;

    httpdmock_start(&m);
    EQI(httpdmock_get(&m, m.url), 200);
    EQS(m.out, "Hello HTTPLOAD!");
    EQS(m.err, "");
    EQI(httpdmock_stop(&m), 0);
}

static void
http10opts(CURL * curl) {
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
}

TEST_CASE void
test_http10_connection(struct test *t) {
    struct curl_slist *headers = NULL;

    struct httpdmock m = {
        .httpd.max_headers_size = 1024
    };

    httpdmock_start(&m);

    headers = curl_slist_append(headers, "Connection: close");
    m.req_headers = headers;
    m.optcb = http10opts;
    EQI(httpdmock_get(&m, m.url), 200);
    EQS(m.out, "Hello HTTPLOAD!");
    EQS(m.err, "");
    EQI(httpdmock_stop(&m), 0);
    curl_slist_free_all(headers);
}

TEST_CASE void
test_http11_connection(struct test *t) {
    struct curl_slist *headers = NULL;

    struct httpdmock m = {
        .httpd.max_headers_size = 1024
    };

    httpdmock_start(&m);

    headers = curl_slist_append(headers, "Connection: close");
    m.req_headers = headers;
    EQI(httpdmock_get(&m, m.url), 200);
    EQS(m.out, "Hello HTTPLOAD!");
    EQS(m.err, "");
    EQI(httpdmock_stop(&m), 0);
    curl_slist_free_all(headers);
}

#define TESTBODY "foo=bar&baz=qux"

static void
bodyopts(CURL * curl) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, TESTBODY);
}

TEST_CASE void
test_body(struct test *t) {
    struct httpdmock m = {
        .httpd.max_headers_size = 1024
    };

    httpdmock_start(&m);
    m.optcb = bodyopts;
    EQI(httpdmock_get(&m, m.url), 200);
    EQS(m.out, TESTBODY);
    EQS(m.err, "");
    EQI(httpdmock_stop(&m), 0);
}

TEST_CASE void
test_body_large(struct test *t) {
    struct httpdmock m = {
        .httpd.max_headers_size = 1024
    };
    size_t len = 1024 + 1;
    char *payload = malloc(len);

    for (int i = 0; i < len; i++) {
        payload[i] = 'a';
    }
    httpdmock_start(&m);
    int status = httpdmock_post(&m, m.url, payload, len);

    EQI(httpdmock_stop(&m), 0);
    EQI(status, 200);
    EQS(m.err, "");
    EQNS(len, m.out, payload);
    free(payload);
}

TEST_CASE void
test_body_verylarge(struct test *t) {
    struct httpdmock m = {
        .httpd.max_headers_size = 1024
    };

    size_t len = EV_WRITE_BUFFSIZE;
    char *payload = malloc(len);

    for (int i = 0; i < len; i++) {
        payload[i] = 'a';
    }
    httpdmock_start(&m);
    int status = httpdmock_post(&m, m.url, payload, len);

    EQI(httpdmock_stop(&m), 0);
    EQS(m.err, "");
    EQS(m.out, "");
    EQI(status, 413);
    free(payload);
}

TEST_CASE void
test_invalid_packet(struct test *t) {
    struct curl_slist *headers = NULL;

    struct httpdmock m = {
        .httpd.max_headers_size = 32
    };

    httpdmock_start(&m);

    headers = curl_slist_append(headers, "Host: BadValue");
    m.req_headers = headers;
    m.optcb = http10opts;
    EQI(httpdmock_get(&m, m.url), 400);
    EQS(m.out, "");
    EQS(m.err, "");
    EQI(httpdmock_stop(&m), 0);
    curl_slist_free_all(headers);

}

#define EXPECTED_STDERR \
    "http-parser: 12: HPE_HEADER_OVERFLOW" \
    N

int
main() {
    struct capt capt;
    struct test *t = TEST_BEGIN(LL_WARN);

    STDCAPT_ERR(capt);
    test_single_packet(t);
    test_http10_connection(t);
    test_http11_connection(t);
    test_body(t);
    test_body_large(t);
    test_body_verylarge(t);
    test_invalid_packet(t);
    EQERR(capt, EXPECTED_STDERR);
    return TEST_CLEAN(t);
}
