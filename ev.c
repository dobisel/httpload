#include "logging.h"
#include "ev.h"
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <netinet/in.h>

static char *tmp;

void
ev_write(struct ev *ev, struct peer *c) {
    ssize_t bytes = 0;

    /* write */
    for (;;) {
        bytes = rb_readf(&c->writerb, c->fd, EV_WRITE_CHUNKSIZE);
        if (bytes <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                c->state = PS_WRITE;
            }
            else if (bytes == 0) {
                ((ev_cb_t) ev->on_writefinish) (ev, c);
            }
            else {
                c->state = PS_CLOSE;
            }
            return;
        }
    }
}

void
ev_read(struct ev *ev, struct peer *c) {
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
                ((ev_recvcb_t) ev->on_recvd) (ev, c, tmp, tmplen);
                return;
            }
            c->state = PS_CLOSE;
            return;
        }
        tmplen += bytes;
    }
}

struct peer *
ev_newconn(struct evs *evs) {
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

    ((ev_cb_t) evs->on_connect) (evs, c);
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
    free(tmp);
    exit(EXIT_SUCCESS);
}

void
ev_fork(struct ev *ev, ev_loop_t loop) {
    pid_t pid;

    if (ev->forks == 0) {
        ERRX("Invalid number of forks: %d", ev->forks);
    }

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

            /* Allocate memory for temp buffer. */
            tmp = malloc(EV_READ_BUFFSIZE);

            /* Set no buffer for stdout */
            //setvbuf(stdout, NULL, _IONBF, 0);

            ev->id = i;
            signal(SIGINT, sigint);
            loop(ev);
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
