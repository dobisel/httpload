#include "logging.h"
#include "ev_common.h"

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <netinet/in.h>

static struct ev *_ev;
static char *_tmp;

void
ev_common_write(struct ev *ev, struct peer *c) {
    ssize_t bytes = 0;

    /* write */
    for (;;) {
        bytes = rb_readf(&c->writerb, c->fd, EV_WRITE_CHUNKSIZE);
        if (bytes <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                c->status = PS_WRITE;
            }
            else if (bytes == 0) {
                ev->on_writefinish(ev, c);
            }
            else {
                c->status = PS_CLOSE;
            }
            return;
        }
    }
}

void
ev_common_read(struct ev *ev, struct peer *c) {
    size_t tmplen = 0;
    ssize_t bytes = 0;

    for (;;) {
        if (tmplen == EV_READ_BUFFSIZE) {
            /* Buffer full */
            c->status = PS_CLOSE;
            return;
        }
        bytes = read(c->fd, _tmp + tmplen, EV_READ_CHUNKSIZE);
        if (bytes <= 0) {
            if (errno == EAGAIN) {
                errno = 0;
                /* Calling read callback */
                ev->on_recvd(ev, c, _tmp, tmplen);
                return;
            }
            c->status = PS_CLOSE;
            return;
        }
        tmplen += bytes;
    }
}

void
ev_common_peer_disconn(struct evs *evs, struct peer *c) {
    if (c == NULL) {
        return;
    }

    if (evs->on_disconnect) {
        evs->on_disconnect(evs, c);
    }

    if (close(c->fd)) {
        WARN("Cannot close fd: %d", c->fd);
    }
    free(c);
}

struct peer *
ev_common_newconn(struct evs *evs) {
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
            return NULL;
        }
        WARN("Error Accepting new connection %d", evs->id);
        return NULL;
    }

    if (fd >= EV_MAXFDS) {
        close(fd);
        WARN("MAX fd exceeded, close(fd), free memory and return.");
        return NULL;
    }

    /* Create and allocate new peer structure. */
    c = malloc(sizeof (struct peer));
    c->fd = fd;
    c->status = PS_UNKNOWN;

    /* Initialize ringbuffer. */
    rb_init(&c->writerb, c->writebuff, EV_WRITE_BUFFSIZE);

    evs->on_connect(evs, c);
    if (c->status == PS_CLOSE) {
        /* Early close. */
        /* Callback returned error, close(fd), free memory and return. */
        close(fd);
        free(c);
        return NULL;
    }
    return c;
}

static void
_parent_sigint(int s) {
    for (int i = 0; i < _ev->forks; i++) {
        kill(_ev->children[i], SIGINT);
    }
}

static void
_child_sigint(int s) {
    _ev->cancel = true;
}

void
ev_common_init(struct ev *ev) {
    /* Allocate memory for temp buffer. */
    _tmp = malloc(EV_READ_BUFFSIZE);

    /* Set no buffer for stdout */
    setvbuf(stdout, NULL, _IONBF, 0);

    ev->cancel = false;
}

void
ev_common_deinit(struct ev *ev) {
    /* Deallocate memory of the temp buffer. */
    free(_tmp);
}

int
ev_common_fork(struct ev *ev, ev_cb_t loop) {
    pid_t pid;
    int ret = OK;

    /* Allocate memory for children pids. */
    ev->children = calloc(ev->forks, sizeof (pid_t));
    if (ev->children == NULL) {
        ERROR("Insifficient memory for %d forks.", ev->forks);
        return ERR;
    }

    /* Preserve struct ev for signal handler. */
    _ev = ev;

    signal(SIGINT, _parent_sigint);
    signal(SIGCHLD, _parent_sigint);

    for (int i = 0; i < ev->forks; i++) {
        pid = fork();
        if (pid == ERR) {
            ERRORX("Cannot fork");  // LCOV_EXCL_LINE
        }
        if (pid > 0) {
            /* Parent */
            ev->children[i] = pid;
        }
        else if (pid == 0) {
            /* Child */
            ev->id = i;

            /* Kill children by parent. */
            prctl(PR_SET_PDEATHSIG, SIGINT);
            signal(SIGINT, _child_sigint);

            /* Initialize ev loop. */
            ev_common_init(ev);

            /* Main loop */
            ret = loop(ev);

            /* Deinitialize ev loop. %d */
            ev_common_deinit(ev);

            exit(ret ? EXIT_FAILURE : EXIT_SUCCESS);
        }
    }
    return ret;
}

int
ev_common_terminate(struct ev *ev) {
    for (int i = 0; i < ev->forks; i++) {
        kill(ev->children[i], SIGINT);
    }
    return ev_common_join(ev);
}

int
ev_common_join(struct ev *ev) {
    int status;
    int ret = 0;

    for (int i = 0; i < ev->forks; i++) {
        waitpid(ev->children[i], &status, 0);
        ret |= WEXITSTATUS(status);
    }

    free(ev->children);
    return ret;
}
