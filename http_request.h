#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string.h>

typedef void (*callback)(int status, char* body, void* arg);

int send_request(
        int fd,
        char* host,
        char* verb,
        char* path,
        char* headers[],
        int header_count,
        char* body,
        callback cb
    );

#endif
