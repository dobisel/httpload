#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string.h>

typedef void (*callback)(int status, char* body, void* arg);

#define HTTP_VERSION "HTTP/1.0"

#define CR "\r\n"

#define VERB_PATH_VERSION(V, P) #V " "  #P " "  HTTP_VERSION CR

int write_verb_path(const int fd, const char* verb, const char* path);

int write_host(const int fd, const char* host);

int write_headers(const int fd, char* headers[], const int header_len);

int write_body(const int fd, const char* body);

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
