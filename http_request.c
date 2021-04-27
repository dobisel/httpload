#include "http_request.h"
#include <unistd.h>
#include <stdio.h>
#include <common.h>

int 
write_headers(const int fd, char *headers[], const int header_count) {
    int i;
    int sum = 0;

    for (i = 0; i < header_count; i++) {
        sum += dprintf(fd, "%s" RN, headers[i]);
    }
    return sum;
}

int 
write_body(const int fd, const char *body, const int body_len) {
    int sum = 0;

    sum += dprintf(fd, "Content-Length: %ld" RN RN, strlen(body));
    sum += write(fd, body, body_len);
    return sum;
}

int 
request_write(int fd, char *host, char *verb, char *path, char *headers[],
        int header_count, char *body, int body_len, char *httpv) {
    int sum = 0;

    sum += dprintf(fd, "%s %s %s" RN, verb, path, httpv);
    sum += dprintf(fd, "HOST: %s" RN, host);
    sum += write_headers(fd, headers, header_count);
    sum += write_body(fd, body, body_len);
    return sum;
}
