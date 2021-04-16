#include "logging.h"
#include "ringbuffer.h"
#include "helpers.h"
#include "ev_epoll.h"
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

static int
server_loop(struct evs *evs) {
    struct epoll_event *events;
    struct epoll_event ev = { 0 };
    struct peer *c;
    int nready;
    int epollfd;
    int op;

    // TODO: Share epollfd between processes or not?
    /* Create epoll. */
    epollfd = epoll_create1(0);
    if (epollfd < 0) {
        ERRX("Cannot create epoll.");
    }

    /* Register accept event */
    ev.data.fd = evs->listenfd;
    ev.events = EPCOMM | EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, evs->listenfd, &ev)) {
        ERRX("epol_ctl error, add listenfd: %d", evs->listenfd);
    }

    events = calloc(EV_BATCHSIZE, sizeof (struct epoll_event));
    if (events == NULL) {
        ERRX("Unable to allocate memory for epoll_events.");
    }

    for (;;) {
        nready = epoll_wait(epollfd, events, EV_BATCHSIZE, -1);

        for (int i = 0; i < nready; i++) {
            if (events[i].data.fd == evs->listenfd) {
                if (events[i].events & EPOLLERR) {
                    WARN("epoll_wait returned EPOLLERR");
                    continue;
                }

                /* Register listenfd for the next connection. */
                ev.data.fd = evs->listenfd;
                ev.events = EPCOMM | EPOLLIN;
                if (epoll_ctl(epollfd, EPOLL_CTL_MOD, evs->listenfd, &ev)) {
                    ERRX("epol_ctl error, add listenfd: %d", evs->listenfd);
                }

                /* New connection */
                c = ev_newconn(evs);
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
                    ev_write((struct ev*)evs, c);
                }
                else if (events[i].events & EPOLLIN) {
                    /* Read */
                    ev_read((struct ev*)evs, c);
                }

                if (c->state == PS_CLOSE) {
                    op = EPOLL_CTL_DEL;
                }
            }

            //DBUG("CTL: %s %s fd: %d", opname(op), statename(c->state), c->fd);
            if (op == EPOLL_CTL_DEL) {
                if (epoll_ctl(epollfd, EPOLL_CTL_DEL, c->fd, NULL)) {
                    ERRX("Cannot DEL EPOLL for fd: %d", c->fd);
                }

                if (close(c->fd)) {
                    WARN("Cannot close fd: %d", c->fd);
                }
                free(c);
            }
            else {
                
                if (c->state == PS_UNKNOWN) {
                    ERRX("Invalid peer state: %s", statename(c->state));
                }
                ev.data.ptr = c;
                ev.events = EPCOMM | 
                    (c->state == PS_WRITE ? EPOLLOUT: EPOLLIN);
                if (epoll_ctl(epollfd, op, c->fd, &ev)) {
                    ERRX("epol_ctl error, op: %d fd: %d", op, c->fd);
                }
            }
        }
    }
    return OK;
}

void
ev_epoll_server_start(struct evs *evs) {
    /* Create and listen tcp socket */
    evs->listenfd = tcp_listen(&(evs->bind));

    ev_fork(evs, (ev_loop_t)server_loop); 
}
