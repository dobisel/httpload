#include "logging.h"
#include "fixtures/assert.h"
#include "fixtures/httpdmock.h"

#include <stdlib.h>

void
test_single_packet(struct test *t) {
    struct httpdmock m;

    httpdmock_start(&m);
    EQI(httpdmock_get(&m), 200);
    EQS(m.out, "Hello HTTPLOAD!");
    EQS(m.err, "");
    EQI(httpdmock_stop(&m), 0);
}

static void
http10opts(CURL * curl) {
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
}

void
test_http10_connection(struct test *t) {
    struct curl_slist *headers = NULL;
    struct httpdmock m = { };

    httpdmock_start(&m);

    headers = curl_slist_append(headers, "Connection: close");
    m.req_headers = headers;
    m.optcb = http10opts;
    EQI(httpdmock_get(&m), 200);
    EQS(m.out, "Hello HTTPLOAD!");
    EQS(m.err, "");
    EQI(httpdmock_stop(&m), 0);
    curl_slist_free_all(headers);
}

void
test_http11_connection(struct test *t) {
    struct curl_slist *headers = NULL;
    struct httpdmock m = { };

    httpdmock_start(&m);

    headers = curl_slist_append(headers, "Connection: close");
    m.req_headers = headers;
    EQI(httpdmock_get(&m), 200);
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

void
test_body(struct test *t) {
    struct httpdmock m;

    httpdmock_start(&m);
    m.optcb = bodyopts;
    EQI(httpdmock_get(&m), 200);
    EQS(m.out, TESTBODY);
    EQS(m.err, "");
    EQI(httpdmock_stop(&m), 0);
}

int
main() {
    struct test t;

    log_setlevel(LL_WARN);
    SETUP(&t);
    test_single_packet(&t);
    test_http10_connection(&t);
    test_http11_connection(&t);
    test_body(&t);
    return TEARDOWN(&t);
}
