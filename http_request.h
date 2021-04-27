#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string.h>


#define HTTP_V1_0 "HTTP/1.0"
#define HTTP_V1_1 "HTTP/1.1"

int write_verb_path_version(const int fd, const char *verb, const char *path,
        const char* httpv);

int write_host(const int fd, const char *host);

int write_headers(const int fd, char *headers[], const int header_len);

int write_body(const int fd, const char *body, const int body_len);

int request_write(int fd, char *host, char *verb, char *path, char *headers[],
        int header_count, char *body, int body_len, char *httpv);

#endif
