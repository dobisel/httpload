/* local */
#include "logging.h"
#include "ringbuffer.h"
#include "ev_common.h"
#include "ev_epoll.h"

/* system */
#include <stdbool.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>

#define EPCOMM (EPOLLONESHOT | EPOLLET)

#define opname(o) ({ \
    char *n; \
    switch (o) {  \
    case EPOLL_CTL_ADD: \
        n = "ADD"; \
        break; \
    case EPOLL_CTL_DEL: \
        n = "DEL"; \
        break; \
    case EPOLL_CTL_MOD: \
        n = "MOD"; \
        break; \
    default: \
        n = "UNKNOWN"; \
        break; \
    }; n;})

#define statename(s) ({ \
    char *n; \
    switch (s) {  \
    case PS_CLOSE: \
        n = "CLOSE"; \
        break; \
    case PS_READ: \
        n = "READ"; \
        break; \
    case PS_WRITE: \
        n = "WRITE"; \
        break; \
    default: \
        n = "UNKNOWN"; \
        break; \
    }; n;})

struct ev_epoll {
    int fd;
    struct epoll_event *events;
};

int
ev_epoll_server_loop(struct evs *evs) {
    struct epoll_event *events;
    struct epoll_event ev = { 0 };
    struct peer *c;
    int epollfd;
    int nready;
    int op;

    /* Create epoll. */
    evs->epoll->fd = epollfd = epoll_create1(EPOLL_CLOEXEC);
    if (epollfd < 0) {
        return ERR;
    }

    /* Register accept event */
    ev.data.fd = evs->listenfd;
    ev.events = EPCOMM | EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, evs->listenfd, &ev)) {
        ERRORX("epol_ctl error, add listenfd: %d", evs->listenfd); // LCOV_EXCL_LINE
    }

    /* Allocate memory for epoll events. */
    evs->epoll->events = events = calloc(EV_BATCHSIZE, 
            sizeof (struct epoll_event));
    if (events == NULL) {
        ERRORX("Unable to allocate memory for epoll_events."); // LCOV_EXCL_LINE
    }

    while (true) {
        nready = epoll_wait(epollfd, events, EV_BATCHSIZE, -1);
        /* nready: %d */
        if (nready == ERR) {
            if (errno == EINTR) {
                return OK;
            }
            ERROR("epoll_wait() returned err(%d)", errno);
            return ERR;
        }

        for (int i = 0; i < nready; i++) {
            if (events[i].events & EPOLLERR) {
                ERROR("epoll_wait returned EPOLLERR");
                return ERR;
            }

            if (events[i].data.fd == evs->listenfd) {
                /* Listenfd triggered: %d */

                /* Register listenfd for the next connection. */
                ev.data.fd = evs->listenfd;
                ev.events = EPCOMM | EPOLLIN;
                if (epoll_ctl(epollfd, EPOLL_CTL_MOD, evs->listenfd, &ev)) {
                    ERROR("epol_ctl error, add listenfd: %d", evs->listenfd);
                    return ERR;
                }

                /* New connection */
                c = ev_common_newconn(evs);
                if (c == NULL) {
                    continue;
                }
                op = EPOLL_CTL_ADD;
            }
            else {
                /* A peer socket is ready for IO. */
                c = (struct peer *) events[i].data.ptr;
                op = EPOLL_CTL_MOD;

                if (events[i].events & EPOLLRDHUP) {
                    op = EPOLL_CTL_DEL;
                }
                else if (events[i].events & EPOLLOUT) {
                    /* Write: %d */
                    ev_common_write((struct ev *) evs, c);
                }
                else if (events[i].events & EPOLLIN) {
                    /* Read */
                    ev_common_read((struct ev *) evs, c);
                }

                if (c->state == PS_CLOSE) {
                    op = EPOLL_CTL_DEL;
                }
            }

            //DBUG("CTL: %s %s fd: %d", opname(op), statename(c->state), c->fd);
            if (op == EPOLL_CTL_DEL) {
                if (epoll_ctl(epollfd, EPOLL_CTL_DEL, c->fd, NULL)) {
                    WARN("Cannot DEL EPOLL for fd: %d", c->fd); 
                    continue;
                }

                ev_common_peer_disconn(evs, c);
            }
            else {

                if (c->state == PS_UNKNOWN) {
                    ERROR("Invalid peer state: %s", statename(c->state));  
                    return ERR;
                }
                ev.data.ptr = c;
                ev.events = EPCOMM |
                    (c->state == PS_WRITE ? EPOLLOUT : EPOLLIN);
                if (epoll_ctl(epollfd, op, c->fd, &ev)) {
                    ERROR("epol_ctl error, op: %d fd: %d", op, c->fd); 
                    return ERR;;
                }
            }
        }
    }
    return ERR;
}

int
ev_epoll_server_init(struct evs *evs) {

    /* Allocate memory for epoll private data. */
    evs->epoll = malloc(sizeof (struct ev_epoll));
    if (evs->epoll == NULL) {
        ERROR("Insufficient memory to allocate for epoll data.");
        return ERR;
    }
    return OK;
}

void
ev_epoll_server_deinit(struct evs *evs) {
    if (evs->epoll) {
        if (evs->epoll->fd > 0) {
            close(evs->epoll->fd);
        }
        if(evs->epoll->events) {
            free(evs->epoll->events);
        }
        free(evs->epoll);
    }
}
