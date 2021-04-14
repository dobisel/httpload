#include "common.h"
#include "options.h"
#include "logging.h"
#include "helpers.h"
#include <fcntl.h>
#include <string.h>

int
enable_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1) {
        return ERR;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return ERR;
    }
    return OK;
}

int
tcp_listen(uint16_t * port) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof (addr);
    int err;
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        return ERR;
    }

    /* Avoid EADDRINUSE. */
    int opt = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) < 0) {
        return ERR;
    }

    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(*port);
    err = bind(fd, (struct sockaddr *) &addr, sizeof (addr));
    if (err) {
        ERROR("cannot bind on: %d", addr.sin_port);
        return ERR;
    }

    if (*port == 0) {
        err = getsockname(fd, (struct sockaddr *) &addr, &addrlen);
        if (err) {
            ERROR("cannot get socketinfo: %d", fd);
            return ERR;
        }
        *port = ntohs(addr.sin_port);
    }

    if (listen(fd, TCP_BACKLOG) < 0) {
        return ERR;
    }

    if (enable_nonblocking(fd)) {
        return ERR;
    }
    return fd;
}
