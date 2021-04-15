#include "http_request.h"
#include <unistd.h>
#include <stdio.h>

int write_verb_path(const int fd, const char* verb, const char* path) {
    return dprintf(fd, "%s %s %s\r\n", verb, path, HTTP_VERSION);
}

int send_request(
        int fd,
        char* host,
        char* verb,
        char* path,
        char* headers[],
        int header_count,
        char* body,
        callback cb
    ) {
    int err, sum = 0;
    err = write_verb_path(fd, verb, path);
    if (err < 0)
        return -1;
    sum += err;
    err = write_host(fd, host);
    if (err < 0)
        return -1;
    sum += err;
    err = write_headers(fd, headers, header_count);
    if (err < 0)
        return -1;
    sum += err;
    err = write_body(fd, body);
    if (err < 0)
        return -1;
    sum += err;
    return sum;
}
