#include "logging.h"
#include "ev_common.h"

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <netinet/in.h>

static char *tmp;

void
ev_common_write(struct ev *ev, struct peer *c) {
    ssize_t bytes = 0;

    /* write */
    for (;;) {
        bytes = rb_readf(&c->writerb, c->fd, EV_WRITE_CHUNKSIZE);
        if (bytes <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                c->state = PS_WRITE;
            }
            else if (bytes == 0) {
                ev->on_writefinish(ev, c);
            }
            else {
                c->state = PS_CLOSE;
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
            c->state = PS_CLOSE;
            return;
        }
        bytes = read(c->fd, tmp + tmplen, EV_READ_CHUNKSIZE);
        if (bytes <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                errno = 0;
                /* Calling read callback */
                ev->on_recvd(ev, c, tmp, tmplen);
                return;
            }
            c->state = PS_CLOSE;
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
    c->state = PS_UNKNOWN;

    /* Initialize ringbuffer. */
    rb_init(&c->writerb, c->writebuff, EV_WRITE_BUFFSIZE);

    evs->on_connect(evs, c);
    if (c->state == PS_CLOSE) {
        /* Early close. */
        /* Callback returned error, close(fd), free memory and return. */
        close(fd);
        free(c);
        return NULL;
    }
    return c;
}

static void
sigint(int s) {
    /* Deallocate memory of the temp buffer. */
    free(tmp);
    exit(EXIT_SUCCESS);
}

void
ev_common_init(struct ev *ev) {
    /* Allocate memory for temp buffer. */
    tmp = malloc(EV_READ_BUFFSIZE);
}

void
ev_common_deinit(struct ev *ev) {
    /* Deallocate memory of the temp buffer. */
    free(tmp);
}

void
ev_common_fork(struct ev *ev, ev_cb_t loop) {
    pid_t pid;

    /* Allocate memory for children pids. */
    ev->children = calloc(ev->forks, sizeof (pid_t));
    if (ev->children == NULL) {
        ERRX("Insifficient memory for %d forks.", ev->forks);
    }

    for (int i = 0; i < ev->forks; i++) {
        pid = fork();
        if (pid == -1) {
            ERRX("Cannot fork");
        }
        if (pid > 0) {
            /* Parent */
            ev->children[i] = pid;
        }
        else if (pid == 0) {
            /* Child */
            prctl(PR_SET_PDEATHSIG, SIGHUP);

            /* Initialize ev loop. */
            ev_common_init(ev);

            /* Set no buffer for stdout */
            //setvbuf(stdout, NULL, _IONBF, 0);

            ev->id = i;
            signal(SIGINT, sigint);
            loop(ev);
        }
    }
}

void
ev_common_terminate(struct ev *ev) {
    int i;

    for (i = 0; i < ev->forks; i++) {
        kill(ev->children[i], SIGINT);
    }
}

int
ev_common_join(struct ev *ev) {
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
