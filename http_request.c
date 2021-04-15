#include "http_request.h"
#include <unistd.h>
#include <stdio.h>
#include <common.h>

int write_verb_path_version(const int fd, const char *verb, const char *path,
        const char* httpv) {
    return dprintf(fd, "%s %s %s" RN, verb, path, httpv);
}

int write_host(const int fd, const char* host) {
    return dprintf(fd, "HOST: %s" RN, host);
}

int write_headers(const int fd, char *headers[], const int header_count) {
    int i;
    int err;
    int sum = 0;
    for (i = 0; i < header_count; i++) {
        err = dprintf(fd, "%s" RN, headers[i]);
        if (err < 0) {
            return ERR;
        }
        sum += err;
    }
    return sum;
}

int write_body(const int fd, const char *body, const int body_len) {
    int err;
    int sum = 0;
    err = dprintf(fd, "Content-Length: %ld" RN RN, strlen(body));
    if (err < 0) {
        return ERR;
    }
    sum += err;
    err = write(fd, body, body_len);
    if (err < 0) {
        return ERR;
    }
    return sum + err;
}

int send_request(int fd, char *host, char *verb, char *path, char *headers[],
        int header_count, char *body, int body_len, callback cb, char *httpv) {
    int err;
    int sum = 0;
    err = write_verb_path_version(fd, verb, path, httpv);
    if (err < 0) {
        return ERR;
    }
    sum += err;
    err = write_host(fd, host);
    if (err < 0) {
        return ERR;
    }
    sum += err;
    err = write_headers(fd, headers, header_count);
    if (err < 0) {
        return ERR;
    }
    sum += err;
    err = write_body(fd, body, body_len);
    if (err < 0) {
        return ERR;
    }
    sum += err;
    return sum;
}
