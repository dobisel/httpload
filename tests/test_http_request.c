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

int
main() {
    test_write_verb_path();
    return EXIT_SUCCESS;
}
