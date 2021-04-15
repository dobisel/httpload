#include "http_request.h"
#include <unistd.h>
#include <stdio.h>

int write_verb_path(const int fd, const char* verb, const char* path) {
    return dprintf(fd, "%s %s %s\r\n", verb, path, HTTP_VERSION);
}

int write_host(const int fd, const char* host) {
    return dprintf(fd, "HOST: %s\r\n", host);
}

int write_headers(const int fd, char* headers[], const int header_count) {
    int i, err, sum = 0;
    for (i = 0; i < header_count; i++) {
        err = dprintf(fd, "%s\r\n", headers[i]);
        if (err < 0)
            return -1;
        sum += err;
    }
    return sum;
}

int write_body(const int fd, const char* body) {
    int err, sum = 0;
    err = dprintf(fd, "Content-Length: %ld\r\n\r\n", strlen(body));
    if (err < 0)
        return -1;
    sum += err;
    err = write(fd, body, strlen(body));
    if (err < 0)
        return -1;
    return sum + err;
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
