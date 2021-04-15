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

void
ev_del_close(struct peer *c) {
    int fd = c->fd;

    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL)) {
        ERRX("Cannot DEL EPOLL for fd: %d", fd);
    }

    free(c);
    if (close(fd)) {
        ERRX("Cannot close fd: %d", fd);
    }
}

void
ev_ctlfd(int fd, int op, uint32_t e) {
    struct epoll_event ev = { 0 };
    ev.data.fd = fd;
    ev.events = EPOLLONESHOT | EPOLLET | e;
    if (epoll_ctl(epollfd, op, fd, &ev)) {
        ERRX("epol_ctl error, op: %d fd: %d", op, fd);
    }
}


void
ev_ctl(struct peer *c, int op, uint32_t e) {
    struct epoll_event ev = { 0 };
    ev.data.ptr = c;
    ev.events = EPOLLONESHOT | EPOLLET | e;
    if (epoll_ctl(epollfd, op, c->fd, &ev)) {
        ERRX("epol_ctl error, op: %d fd: %d", op, c->fd);
    }
}

static void
readfd(struct ev *ev, struct peer *c) {
    size_t tmplen = 0;
    ssize_t bytes = 0;

    for (;;) {
        bytes = read(c->fd, tmp, EV_READ_CHUNKSIZE);
        if (bytes <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                errno = 0;
                /* RED: %d EAGAIN: %d */
                if (((ev_recvcb_t) ev->on_recvd) (ev, c, tmp, tmplen)) {
                    /* del: %d %d */
                    ev_del_close(c);
                }
                return;
            }
            /* del: %d %d */
            ev_del_close(c);
            return;
        }
        tmplen += bytes;
    }
}

static void
io(struct ev *ev, struct epoll_event e) {
    ssize_t bytes = 0;
    struct peer *c = (struct peer *) e.data.ptr;

    if (e.events & EPOLLRDHUP) {
        ev_del_close(c);
    }
    else if (e.events & EPOLLIN) {
        /* Read: %d */
        readfd(ev, c);
    }
    else if (e.events & EPOLLOUT) {
        /* Write %d */
        for (;;) {
            bytes = rb_readf(&c->resprb, c->fd, EV_WRITE_CHUNKSIZE);
            if (bytes == 0) {
                if (((ev_cb_t) ev->on_writefinish) (ev, c)) {
                    ev_del_close(c);
                }
                else {
                    EV_MOD_READ(c);
                }
                return;
            }
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                EV_MOD_WRITE(c);
                return;
            }
            if (bytes <= 0) {
                ev_del_close(c);
                return;
            }
        }
    }
}

static void
newconnection(struct evs *evs) {
    struct sockaddr_in peeraddr;
    socklen_t peeraddr_len;
    int fd;
    struct peer *c;

    peeraddr_len = sizeof (peeraddr);
    fd = accept4(evs->listenfd, (struct sockaddr *) &peeraddr, &peeraddr_len,
            SOCK_NONBLOCK);
    if (fd < 0) {
        /* Accept error */
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* Accept returned EAGAIN or EWOULDBLOCK %d */
            return;
        }
        ERRX("Error Accepting new connection %d", evs->id);
    }

    if (fd >= EV_MAXFDS) {
        ERRX("fd %d >= EV_MAXFDS (%d)", fd, EV_MAXFDS);
    }

    /* Create and allocate new peer structure. */
    c = malloc(sizeof (struct peer));
    c->fd = fd;
    c->len = 0;

    /* Initialize ringbuffer. */
    rb_init(&c->resprb, c->buff, EV_WRITE_BUFFSIZE);

    if (((ev_cb_t) evs->on_connect) (evs, c)) {
        CHK("Callback returned error, free memory and return.");
        free(c);
        return;
    }

    /* Register new peer into event loop. %d */
    EV_ADD_READ(c);
}

void
sigint(int s) {
    free(tmp);
    exit(EXIT_SUCCESS);
}

static int
loop(struct evs *evs) {
    struct epoll_event *events;

    signal(SIGINT, sigint);

    /* Allocate memory for temp buffer. */
    tmp = malloc(EV_READ_BUFFSIZE);

    /* Create epoll. */
    epollfd = epoll_create1(0);
    if (epollfd < 0) {
        ERRX("Cannot create epoll.");
    }

    /* Register accept event */
    EV_ADD_READ_FD(evs->listenfd);

    events = calloc(EV_BATCHSIZE, sizeof (struct epoll_event));
    if (events == NULL) {
        ERRX("Unable to allocate memory for epoll_events.");
    }

    for (;;) {
        int nready = epoll_wait(epollfd, events, EV_BATCHSIZE, -1);

        for (int i = 0; i < nready; i++) {
            if (events[i].data.fd == evs->listenfd) {
                if (events[i].events & EPOLLERR) {
                    DBUG("epoll_wait returned EPOLLERR");
                }
                else {
                    /* New connection: %d fork: %d */
                    newconnection(evs);
                }
                EV_MOD_READ_FD(evs->listenfd);
            }
            else {
                /* A peer socket is ready. */
                io(evs, events[i]);
            }
        }
    }
}

void
evs_fork(struct evs *evs) {
    int i;
    pid_t pid;

    /* Create and listen tcp socket */
    evs->listenfd = tcp_listen(&(evs->port));

    if (evs->forks == 0) {
        ERRX("Invalid number of forks: %d", evs->forks);
    }

    /* Allocate memory for children pids. */
    evs->children = calloc(evs->forks, sizeof (pid_t));
    if (evs->children == NULL) {
        ERRX("Insifficient memory for %d forks.", evs->forks);
    }

    for (i = 0; i < evs->forks; i++) {
        pid = fork();
        if (pid == -1) {
            ERRX("Cannot fork");
        }
        if (pid > 0) {
            /* Parent */
            evs->children[i] = pid;
        }
        else if (pid == 0) {
            /* Child */
            prctl(PR_SET_PDEATHSIG, SIGHUP);

            /* Set no buffer for stdout */
            //setvbuf(stdout, NULL, _IONBF, 0);
            evs->id = i;
            loop(evs);
        }
    }
}

void
ev_terminate(struct ev *ev) {
    int i;

    for (i = 0; i < ev->forks; i++) {
        kill(ev->children[i], SIGINT);
    }
}

int
ev_join(struct ev *ev) {
    int status;
    int ret = 0;
    int i;

    for (i = 0; i < ev->forks; i++) {
        waitpid(ev->children[i], &status, 0);
        ret |= WEXITSTATUS(status);
    }

    free(ev->children);
    return ret;
}
