#include "logging.h"
#include "ringbuffer.h"
#include "helpers.h"
#include "ev.h"
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>

// TODO: EPOLLRDHUP

static char *tmp;
static int epollfd;

int
ev_want_close(struct peer *c) {
    int err;
    int fd = c->fd;

    err = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
    if (err) {
        ERROR("Cannot DEL EPOLL for: %d", fd);
        return err;
    }
    free(c);
    return close(fd);
}

int
ev_want(struct peer *c, int op, uint32_t e) {
    struct epoll_event ev = { 0 };
    ev.data.ptr = c;
    ev.events = EPOLLONESHOT | EPOLLET | e;
    return epoll_ctl(epollfd, op, c->fd, &ev);
}

static int
io(struct ev *ev, struct epoll_event e) {
    size_t tmplen = 0;
    ssize_t bytes = 0;
    struct peer *c = (struct peer *) e.data.ptr;

    if (e.events & EPOLLRDHUP) {
        /* Closed */
        if (EV_MOD_CLOSE(c)) {
            ERROR("Cannot close: %d", c->fd);
        }
    }
    else if (e.events & EPOLLIN) {
        /* Read */
        for (;;) {
            bytes = read(c->fd, tmp, EV_READSIZE);
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                errno = 0;
                if (((ev_recvcb_t) ev->on_recvd) (ev, c, tmp, tmplen)) {
                    if (EV_MOD_CLOSE(c)) {
                        ERROR("Cannot close: %d", c->fd);
                    }
                }
                break;
            }
            if (bytes <= 0) {
                if (EV_MOD_CLOSE(c)) {
                    ERROR("Cannot close: %d", c->fd);
                }
                break;
            }
            tmplen += bytes;
        }
    }
    else if (e.events & EPOLLOUT) {
        /* Write */
        for (;;) {
            bytes = rb_readf(&c->resprb, c->fd, EV_WRITE_CHUNKSIZE);
            if (bytes == 0) {
                if ((((ev_cb_t) ev->on_writefinish) (ev, c)) ||
                    EV_MOD_READ(c)) {
                    if (EV_MOD_CLOSE(c)) {
                        ERROR("Cannot close: %d", c->fd);
                    }
                }
                break;
            }
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                EV_MOD_WRITE(c);
                break;
            }
            if (bytes <= 0) {
                if (EV_MOD_CLOSE(c)) {
                    ERROR("Cannot close: %d", c->fd);
                }
                break;
            }
        }
    }
    return OK;
}

static int
newconnection(struct evs *evs) {
    struct sockaddr_in peeraddr;
    socklen_t peeraddr_len;
    int fd;
    struct peer *c;

    peeraddr_len = sizeof (peeraddr);
    fd = accept(evs->listenfd, (struct sockaddr *) &peeraddr, &peeraddr_len);
    if (fd < 0) {
        /* Accept error */
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            DEBUG("accept returned EAGAIN or EWOULDBLOCK\n");
            return OK;
        }
        else {
            ERROR("accept");
        }
        return ERR;
    }

    if (enable_nonblocking(fd)) {
        ERROR("Cannot enable nonblocking mode for fd: %d", fd);
        return ERR;;
    }

    if (fd >= EV_MAXFDS) {
        ERROR("fd (%d) >= EV_MAXFDS (%d)", fd, EV_MAXFDS);
        return ERR;
    }

    /* Create and allocate new peer structure. */
    c = malloc(sizeof (struct peer));
    c->fd = fd;
    c->len = 0;

    /* Initialize ringbuffer. */
    rb_init(&c->resprb, c->buff, EV_WRITEBUFF_SIZE);

    if (((ev_cb_t) evs->on_connect) (evs, c)) {
        free(c);
        return ERR;
    }

    if (EV_ADD_READ(c)) {
        ERROR("Cannot register fd: %d for read", fd);
        return ERR;
    }

    return OK;
}

void
sigint(int s) {
    free(tmp);
    exit(EXIT_SUCCESS);
}

static int
loop(struct evs *evs) {
    int err;

    signal(SIGINT, sigint);

    /* Allocate memory for temp buffer. */
    tmp = malloc(EV_READSIZE);

    epollfd = epoll_create1(0);
    if (epollfd < 0) {
        ERROR("Creating epoll");
        return epollfd;
    }

    /* Accept event */
    struct epoll_event epv;

    epv.data.fd = evs->listenfd;
    epv.events = EPOLLIN;
    err = epoll_ctl(epollfd, EPOLL_CTL_ADD, evs->listenfd, &epv);
    if (err < 0) {
        ERROR("epoll_ctl EPOLL_CTL_ADD");
        return err;
    }

    struct epoll_event *events =
        calloc(EV_MAXFDS, sizeof (struct epoll_event));

    if (events == NULL) {
        ERROR("Unable to allocate memory for epoll_events");
        return ERR;
    }

    for (;;) {
        int nready = epoll_wait(epollfd, events, EV_MAXFDS, -1);

        for (int i = 0; i < nready; i++) {
            if (events[i].events & EPOLLERR) {
                DEBUG("epoll_wait returned EPOLLERR");
            }

            if (events[i].data.fd == evs->listenfd) {
                /* The listening socket is ready; 
                 * this means a new peer is connecting.
                 */
                if (newconnection(evs)) {
                    ERROR("Accepting new connection");
                    return ERR;
                }
            }
            else {
                /* A peer socket is ready. */
                if (io(evs, events[i])) {
                    ERROR("Processing socket IO.");
                    return ERR;
                }
            }
        }
    }
    return OK;
}

int
evs_fork(struct evs *m) {
    int i;
    pid_t pid;

    /* Create and listen tcp socket */
    m->listenfd = tcp_listen(&(m->port));
    if (m->listenfd < 0) {
        ERROR("Listening socket.");
        return ERR;
    }

    if (m->forks == 0) {
        ERROR("Invalid number of forks: %d", m->forks);
        return ERR;
    }

    /* Allocate memory for children pids. */
    m->children = calloc(m->forks, sizeof (pid_t));
    if (m->children == NULL) {
        return ERR;
    }

    for (i = 0; i < m->forks; i++) {
        pid = fork();
        if (pid == -1) {
            ERROR("Cannot fork");
            return ERR;
        }
        if (pid > 0) {
            /* Parent */
            m->children[i] = pid;
        }
        else if (pid == 0) {
            /* Child */
            prctl(PR_SET_PDEATHSIG, SIGHUP);

            /* Set no buffer for stdout */
            setvbuf(stdout, NULL, _IONBF, 0);

            return loop(m);
        }
    }
    return OK;
}

void
ev_terminate(struct ev *m) {
    int i;

    for (i = 0; i < m->forks; i++) {
        kill(m->children[i], SIGINT);
    }
}

int
ev_join(struct ev *m) {

    int status;
    int ret = 0;
    int i;

    for (i = 0; i < m->forks; i++) {
        waitpid(m->children[i], &status, 0);
        ret |= WEXITSTATUS(status);
    }

    free(m->children);
    return ret;
}
