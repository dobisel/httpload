#include "common.h"
#include "options.h"
#include "logging.h"
#include "helpers.h"
#include <fcntl.h>
#include <string.h>

int
enable_nonblocking(int fd) {
    int flags; 

    flags = fcntl(fd, F_GETFL, 0);
    if (flags != ERR) {
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    return ERR;
}

int
tcp_listen(uint16_t * port) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof (addr);
    int opt;
    int fd;

    /* Create socket. */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        ERRORX("Cannot create socket(AF_INET, ...)");  // LCOV_EXCL_LINE
    }

    /* Avoid EADDRINUSE. */
    opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) < 0) {
        ERRORX("Cannot set socket options for fd: %d", fd);  // LCOV_EXCL_LINE
    }

    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(*port);
    if (bind(fd, (struct sockaddr *) &addr, sizeof (addr))) {
        ERRORX("bind() failed for fd: %d", fd);  // LCOV_EXCL_LINE
    }

    if (*port == 0) {
        if (getsockname(fd, (struct sockaddr *) &addr, &addrlen)) {
            ERRORX("Cannot resolve socket name for fd: %d", fd);  // LCOV_EXCL_LINE
        }
        *port = ntohs(addr.sin_port);
    }

    if (listen(fd, TCP_BACKLOG) < 0) {
        ERRORX("listen() failed for fd: %d", fd);  // LCOV_EXCL_LINE
    }

    if (enable_nonblocking(fd) == ERR) {
        ERRORX("Cannot enable non-blocking for fd: %d", fd);  // LCOV_EXCL_LINE
    }
    return fd;
}
