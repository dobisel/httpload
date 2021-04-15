#include "fixtures/assert.h"
#include "logging.h"
#include "http_request.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define TMP_F tempnam("/tmp/", NULL)

#define MOCKFD open(TMP_F, O_RDWR | O_CREAT, S_IRWXU)

#define CLEANFD remove(TMP_F)

void
test_write_verb_path_version() {
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
    eqstr(vpv, response);
    free(response);
    CLEANFD;
}

void
test_write_host() {
    int fd;
    int len;
    char *host = "google.com";
    char *response;
    fd = MOCKFD;
    len = write_host(fd, host);
    notequalint(ERR, len);
    response = (char*) malloc(len * sizeof(char));
    lseek(fd, 0, SEEK_SET);
    read(fd, response, len);
    eqstr("HOST: google.com" RN, response);
    free(response);
    CLEANFD;
}

void
test_write_headers() {
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
    notequalint(ERR, len);
    lseek(fd, 0, SEEK_SET);
    response = (char*) malloc(len * sizeof(char));
    expected = (char*) malloc(len * sizeof(char));
    read(fd, response, len);
    for (i = 0; i < header_count; i++) {
        strcat(expected, headers[i]);
        strcat(expected, "\r\n");
    }
    eqstr(expected, response);
    free(response);
    free(expected);
    CLEANFD;
}

void
test_write_body() {
    int fd;
    int len;
    char *response;
    char *expected;
    char *body = "foo baz bar cux";
    fd = MOCKFD;
    len = write_body(fd, body, 15);
    notequalint(ERR, len);
    lseek(fd, 0, SEEK_SET);
    response = (char*) malloc(len * sizeof(char));
    expected = (char*) malloc(len * sizeof(char));
    sprintf(expected, "Content-Length: %ld\r\n\r\n%s", strlen(body), body);
    read(fd, response, len);
    eqstr(expected, response);
    free(response);
    free(expected);
    CLEANFD;
}

int
main() {
    log_setlevel(LL_DEBUG);
    test_write_verb_path_version();
    test_write_host();
    test_write_headers();
    test_write_body();
    return EXIT_SUCCESS;
}
