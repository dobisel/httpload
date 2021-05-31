#include "fixtures/assert.h"
#include "http_request.h"

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define TMP_F tempnam("/tmp/", NULL)

#define MOCKFD open(TMP_F, O_RDWR | O_CREAT, S_IRWXU)

#define CLEANFD remove(TMP_F)

void
test_write_verb_path_version(struct test *t) {
    int fd;
    int len;
    char *vpv;
    char *response;
    vpv = "GET /index.html " HTTP_V1_1 RN;
    len = strlen(vpv);
    response = (char*) malloc(len * sizeof(char));
    fd = MOCKFD;
    write_verb_path_version(fd, "GET", "/index.html", HTTP_V1_1);
    lseek(fd, 0, SEEK_SET);
    read(fd, response, len);
    EQS(vpv, response);
    free(response);
    CLEANFD;
}

void
test_write_host(struct test *t) {
    int fd;
    int len;
    char *host = "google.com";
    char *response;
    fd = MOCKFD;
    len = write_host(fd, host);
    NEQI(ERR, len);
    response = (char*) malloc(len * sizeof(char));
    lseek(fd, 0, SEEK_SET);
    read(fd, response, len);
    EQS("HOST: google.com" RN, response);
    free(response);
    CLEANFD;
}

void
test_write_headers(struct test *t) {
    int fd;
    int len;
    int header_count;
    int i;
    char *response;
    char *expected;
    char *headers[] = {
        "Accept-Language: en-US,en;q=0.5",
        "Accept-Encoding: gzip, deflate, br",
        "Referer: https://developer.mozilla.org/testpage.html",
        "Connection: keep-alive",
        "Upgrade-Insecure-Requests: 1"
    };
    header_count = 5;
    fd = MOCKFD;
    len = write_headers(fd, headers, header_count);
    NEQI(ERR, len);
    lseek(fd, 0, SEEK_SET);
    response = (char*) malloc(len * sizeof(char));
    expected = (char*) malloc(len * sizeof(char));
    read(fd, response, len);
    for (i = 0; i < header_count; i++) {
        strcat(expected, headers[i]);
        strcat(expected, "\r\n");
    }
    EQS(expected, response);
    free(response);
    free(expected);
    CLEANFD;
}

void
test_write_body(struct test *t) {
    int fd;
    int len;
    char *response;
    char *expected;
    char *body = "foo baz bar cux";
    fd = MOCKFD;
    len = write_body(fd, body, 15);
    NEQI(ERR, len);
    lseek(fd, 0, SEEK_SET);
    response = (char*) malloc(len * sizeof(char));
    expected = (char*) malloc(len * sizeof(char));
    sprintf(expected, "Content-Length: %ld\r\n\r\n%s", strlen(body), body);
    read(fd, response, len);
    EQS(expected, response);
    free(response);
    free(expected);
    CLEANFD;
}

void
test_send_request(struct test *t) {
    int fd;
    int len;
    char *response;
    char *expected = "GET /foo/baz HTTP/1.1\r\nHOST: google.com\r\n"
        "Connection: keep-alive\r\nContent-Length: 15\r\n\r\nfoo baz bar cux";
    char *headers[] = {
        "Connection: keep-alive",
    };
    fd = MOCKFD;
    len = request_write(fd, "google.com", "GET", "/foo/baz", headers, 1, 
            "foo baz bar cux", 15, HTTP_V1_1);
    NEQI(ERR, len);
    lseek(fd, 0, SEEK_SET);
    response = (char*) malloc(len * sizeof(char));
    read(fd, response, len);
    EQS(expected, response);
    free(response);
    CLEANFD;
}

int
main() {
    struct test *t = TEST_BEGIN(LL_WARN);

    test_write_verb_path_version(t);
    test_write_host(t);
    test_write_headers(t);
    test_write_body(t);
    test_send_request(t);
    return TEST_CLEAN(t);
}
