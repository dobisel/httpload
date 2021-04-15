#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string.h>

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
