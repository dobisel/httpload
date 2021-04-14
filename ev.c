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
ev_ctl(struct peer *c, int op, uint32_t e) {
    struct epoll_event ev = { 0 };
    ev.data.ptr = c;
    ev.events = EPOLLONESHOT | EPOLLET | e;
    if (epoll_ctl(epollfd, op, c->fd, &ev)) {
        ERRX("epol_ctl error, fd: %d", c->fd);
    }
}

static void
io(struct ev *ev, struct epoll_event e) {
    size_t tmplen = 0;
    ssize_t bytes = 0;
    struct peer *c = (struct peer *) e.data.ptr;

    if (e.events & EPOLLRDHUP) {
        ev_del_close(c);
    }
    else if (e.events & EPOLLIN) {
        /* Read */
        for (;;) {
            bytes = read(c->fd, tmp, EV_READ_CHUNKSIZE);
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                errno = 0;
                if (((ev_recvcb_t) ev->on_recvd) (ev, c, tmp, tmplen)) {
                    ev_del_close(c);
                    return;
                }
                return;
            }
            if (bytes <= 0) {
                ev_del_close(c);
                return;
            }
            tmplen += bytes;
        }
    }
    else if (e.events & EPOLLOUT) {
        /* Write */
        for (;;) {
            bytes = rb_readf(&c->resprb, c->fd, EV_WRITE_CHUNKSIZE);
            if (bytes == 0) {
                if (((ev_cb_t) ev->on_writefinish) (ev, c)) {
                    ev_del_close(c);
                }
                EV_MOD_READ(c);
            }
            else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                EV_MOD_WRITE(c);
            }
            else if (bytes <= 0) {
                ev_del_close(c);
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
    fd = accept(evs->listenfd, (struct sockaddr *) &peeraddr, &peeraddr_len);
    if (fd < 0) {
        /* Accept error */
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            DBUG("accept returned EAGAIN or EWOULDBLOCK");
            return;
        }
        ERRX("Accepting new connection");
    }

    if (enable_nonblocking(fd)) {
        ERRX("Cannot enable nonblocking mode for fd: %d", fd);
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
        /* Callback returned error, free memory and return. */
        free(c);
        return;
    }

    /* Register new peer into event loop. */
    EV_ADD_READ(c);
}

void
sigint(int s) {
    free(tmp);
    exit(EXIT_SUCCESS);
}

static int
loop(struct evs *evs) {
    struct epoll_event epv;
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
    epv.data.fd = evs->listenfd;
    epv.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, evs->listenfd, &epv)) {
        ERRX("epoll_ctl EPOLL_CTL_ADD.");
    }

    events = calloc(EV_BATCHSIZE, sizeof (struct epoll_event));
    if (events == NULL) {
        ERRX("Unable to allocate memory for epoll_events.");
    }

    for (;;) {
        int nready = epoll_wait(epollfd, events, EV_BATCHSIZE, -1);

        for (int i = 0; i < nready; i++) {
            if (events[i].events & EPOLLERR) {
                DBUG("epoll_wait returned EPOLLERR");
            }

            if (events[i].data.fd == evs->listenfd) {
                /* New connection */
                newconnection(evs);
            }
            else {
                /* A peer socket is ready. */
                io(evs, events[i]);
            }
        }
    }
}

void
evs_fork(struct evs *m) {
    int i;
    pid_t pid;

    /* Create and listen tcp socket */
    m->listenfd = tcp_listen(&(m->port));

    if (m->forks == 0) {
        ERRX("Invalid number of forks: %d", m->forks);
    }

    /* Allocate memory for children pids. */
    m->children = calloc(m->forks, sizeof (pid_t));
    if (m->children == NULL) {
        ERRX("Insifficient memory for %d forks.", m->forks);
    }

    for (i = 0; i < m->forks; i++) {
        pid = fork();
        if (pid == -1) {
            ERRX("Cannot fork");
        }
        if (pid > 0) {
            /* Parent */
            m->children[i] = pid;
        }
        else if (pid == 0) {
            /* Child */
            prctl(PR_SET_PDEATHSIG, SIGHUP);

            /* Set no buffer for stdout */
            //setvbuf(stdout, NULL, _IONBF, 0);

            loop(m);
        }
    }
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
