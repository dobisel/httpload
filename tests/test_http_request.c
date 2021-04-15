#include "fixtures/assert.h"
#include "logging.h"
#include "http_request.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int 
mock_fd() {
    return open(PATH, O_RDWR | O_CREAT, S_IRWXU);
}

int 
clean_fd() {
    return remove(PATH);
}

void
test_write_verb_path() {
    int fd, len;
    char *vpv, *response;
    vpv = VERB_PATH_VERSION(GET, /index.html);
    len = strlen(vpv);
    response = (char*) malloc(len * sizeof(char));
    fd = mock_fd();
    write_verb_path(fd, "GET", "/index.html");
    lseek(fd, 0, SEEK_SET);
    read(fd, response, len);
    eqstr(vpv, response);
    free(response);
    clean_fd();
}

void
test_write_host() {
    int fd, len;
    char* host = "google.com";
    char *response, *expected;
    fd = mock_fd();
    len = write_host(fd, host);
    notequalint(-1, len);
    expected = (char*) malloc(len * sizeof(char));
    sprintf(expected, "HOST: %s\r\n", host);
    response = (char*) malloc(len * sizeof(char));
    lseek(fd, 0, SEEK_SET);
    read(fd, response, len);
    eqstr(expected, response);
    clean_fd();
}

void
test_write_headers() {
    int fd, len, header_count, i;
    char* response;
    char* expected;
    char* headers[] = {
        "Accept-Language: en-US,en;q=0.5",
        "Accept-Encoding: gzip, deflate, br",
        "Referer: https://developer.mozilla.org/testpage.html",
        "Connection: keep-alive",
        "Upgrade-Insecure-Requests: 1"
    };
    header_count = 5;
    fd = mock_fd();
    len = write_headers(fd, headers, header_count);
    notequalint(-1, len);
    lseek(fd, 0, SEEK_SET);
    response = (char*) malloc(len * sizeof(char));
    expected = (char*) malloc(len * sizeof(char));
    read(fd, response, len);
    for (i = 0; i < header_count; i++) {
        strcat(expected, headers[i]);
        strcat(expected, "\r\n");
    }
    eqstr(expected, response);
    clean_fd();
}

void
test_write_body() {
    int fd, len;
    char* response;
    char* expected;
    char* body = "{\"name\": \"Dennis\", \"lastname\": \"Ritchie\"}";
    fd = mock_fd();
    len = write_body(fd, body);
    notequalint(-1, len);
    lseek(fd, 0, SEEK_SET);
    response = (char*) malloc(len * sizeof(char));
    expected = (char*) malloc(len * sizeof(char));
    sprintf(expected, "Content-Length: %ld\r\n\r\n%s", strlen(body), body);
    read(fd, response, len);
    eqstr(expected, response);
    clean_fd();
}

int
main() {
    test_write_verb_path();
    test_write_host();
    test_write_headers();
    test_write_body();
    return EXIT_SUCCESS;
}
