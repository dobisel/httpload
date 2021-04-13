#include "logging.h"
#include "testing.h"
#include "fixtures/httpdmock.h"

void
test_single_packet() {
    struct httpdmock m;

    httpdmock_start(&m);
    eqint(200, httpdmock_get(&m));
    eqstr("Hello HTTPLOAD!", m.out);
    eqstr("", m.err);
    httpdmock_stop(&m);
}

static void
http10opts(CURL * curl) {
    DEBUG("C OPTS");
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
}

void
test_http10_connection() {
    struct curl_slist *headers = NULL;
    struct httpdmock m = { };

    httpdmock_start(&m);

    headers = curl_slist_append(headers, "Connection: close");
    m.req_headers = headers;
    m.optcb = http10opts;
    eqint(200, httpdmock_get(&m));
    eqstr("Hello HTTPLOAD!", m.out);
    eqstr("", m.err);
    httpdmock_stop(&m);
    curl_slist_free_all(headers);
}

void
test_http11_connection() {
    struct curl_slist *headers = NULL;
    struct httpdmock m = { };

    httpdmock_start(&m);

    headers = curl_slist_append(headers, "Connection: close");
    m.req_headers = headers;
    eqint(200, httpdmock_get(&m));
    eqstr("Hello HTTPLOAD!", m.out);
    eqstr("", m.err);
    httpdmock_stop(&m);
    curl_slist_free_all(headers);
}

int
main() {
    log_setlevel(LL_DEBUG);
    test_single_packet();
    test_http10_connection();
    test_http11_connection();
    return 0;
}
