#include "logging.h"
#include "server.h"
#include "ringbuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <netinet/in.h>
#include <http_parser.h>

#define MAXFDS              1024

#define TMPSIZE           1024 * 64
#define BACKLOG             16
#define OK                   0
#define ERR_LISTEN          -2
#define ERR_ENNONBLOCK      -3
#define ERR_MEM             -4
#define ERR_ACCEPT          -5
#define ERR_MAXFD           -6
#define ERR_FORK            -7
#define ERR_ARGS            -8

// Must be power of 2
#define RESPRB_SIZE          1024 * 64  // 2 ** 16

#define HTTPRESP \
    "HTTP/1.1 %d %s" RN \
    "Server: httpload/" HTTPLOAD_VERSION RN \
    "Content-Length: %ld" RN \
    "Connection: %s" RN


static char *tmp;
static int epollfd;


struct client {
    int fd;
    char buff[RESPRB_SIZE];
    size_t len;
    struct ringbuffer resprb;
    http_parser hp;
};

// TODO: EPOLLRDHUP


static int
want_close(struct client *c) {
    DEBUG("Peer Closed: %d", c->fd);
    int fd = c->fd;
    int err = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
    if (err) {
        ERROR("Cannot DEL EPOLL for: %d", fd);
        return err;
    }
    free(c);
    return close(fd);
}


static int
want(struct client *c, int op, uint32_t e) {
    struct epoll_event ev = { 0 };
    ev.data.ptr = c;
    ev.events = EPOLLONESHOT | EPOLLET | e;
    return epoll_ctl(epollfd, op, c->fd, &ev);
}


#define WANT_READ(c, add) ({ \
    DEBUG("WANT: READ"); \
    want((c), (add)? EPOLL_CTL_ADD: EPOLL_CTL_MOD, EPOLLIN); })

#define WANT_WRITE(c) ({ \
    DEBUG("WANT: WRITE") \
    want((c), EPOLL_CTL_MOD, EPOLLOUT); })


static int
headercomplete_cb(http_parser * p) {
    int err;
    size_t tmplen;
    long clen = p->content_length;
    struct client *c = (struct client *)p->data;
    int keep = http_should_keep_alive(&c->hp);

    tmplen = sprintf(tmp, HTTPRESP, 
            200, "OK", 
            clen > 0? clen: 15,
            keep? "keep-alive": "close"
        );
    err = rb_write(&c->resprb, tmp, tmplen);
    if (err) {
        return err;
    }
    
    /* Write more headers */
    // ....

    /* Terminate headers by `\r\n` */
    err = rb_write(&c->resprb, RN, 2);
    if (err) {
        return err;
    }
    if (clen <= 0) {
        tmplen = sprintf(tmp, "Hello HTTPLOAD!");
        return rb_write(&c->resprb, tmp, tmplen);
    }
    return OK;
}

static int
complete_cb(http_parser * p) {
    struct client *c = (struct client *)p->data;
    DEBUG("Complete: %s", tmp);
    return WANT_WRITE(c);
}

static int
body_cb(http_parser * p, const char *at, size_t len) {
    struct client *c = (struct client *)p->data;
    DEBUG("BODY: %.*s", (int)len, at);
    int err = rb_write(&c->resprb, at, len);
    if (err) {
        ERROR("Buffer full");
        return err;
    }
    return WANT_WRITE(c);
}


static http_parser_settings hpconf = {
    .on_headers_complete = headercomplete_cb,
    .on_body = body_cb,
    .on_message_complete = complete_cb,
};


static int
io(struct epoll_event ev) {
    int err;
    size_t tmplen = 0;
    ssize_t bytes = 0;
    struct client *c = (struct client *) ev.data.ptr;
    if (ev.events & EPOLLRDHUP) {
        /* Closed */
        err = want_close(c);
        if (err) {
            ERROR("Cannot close: %d", c->fd);
        }
    }
    else if (ev.events & EPOLLIN) {
        /* Read */
        DEBUG("Start reading");
        for (;;) {
            bytes = read(c->fd, tmp, TMPSIZE);
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                DEBUG("AGAIN, TOT: %lu", tmplen);
                errno = 0;
                if (http_parser_execute(&c->hp, &hpconf, tmp, tmplen) !=
                        tmplen ) {
                    want_close(c);
                }
                else if (c->hp.http_errno) {
                    DEBUG("http-parser: %d: %s", c->hp.http_errno, 
                            http_errno_name(c->hp.http_errno));
                    want_close(c);
                }
                break;
            }
            if (bytes <= 0) {
                want_close(c);
                break;
            }
            tmplen += bytes;
        }
    }
    else if (ev.events & EPOLLOUT) {
        /* Write */
        for (;;) {
            bytes = rb_readf(&c->resprb, c->fd, RESPRB_SIZE);
            if (bytes == 0) {
                int k = http_should_keep_alive(&c->hp);
                DEBUG("Keep: %d", k);
                if (k) {
                    WANT_READ(c, false);
                }
                else {
                    want_close(c);
                }
                break;
            }
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                DEBUG("Write Again: %lu", tmplen);
                WANT_WRITE(c);
                break;
            }
            if (bytes <= 0) {
                want_close(c);
                break;
            }
        }
    }
    return OK;
}


static int
enable_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return ERR_ENNONBLOCK;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return ERR_ENNONBLOCK;
    }
    return OK;
}


static int
tcp_listen(uint16_t * port) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int err;
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        return ERR_LISTEN;
    }

    /* Avoid EADDRINUSE. */
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return ERR_LISTEN;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(*port);
    err = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
    if (err) {
        ERROR("cannot bind on: %d", addr.sin_port);
        return ERR_LISTEN;
    }

    if (*port == 0) {
        err = getsockname(fd, (struct sockaddr *) &addr, &addrlen);
        if (err) {
            ERROR("cannot get socketinfo: %d", fd);
            return ERR_LISTEN;
        }
        *port = ntohs(addr.sin_port);
    }

    if (listen(fd, BACKLOG) < 0) {
        return ERR_LISTEN;
    }

    if (enable_nonblocking(fd)) {
        return ERR_LISTEN;
    }
    return fd;
}


static int
newconnection(struct epoll_event *event) {
    int err;
    struct sockaddr_in peeraddr;
    socklen_t peeraddr_len;
    int fd;

    peeraddr_len = sizeof(peeraddr);
    fd = accept(event->data.fd, (struct sockaddr *) &peeraddr, &peeraddr_len);

    if (fd < 0) {
        /* Accept error */
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            DEBUG("accept returned EAGAIN or EWOULDBLOCK\n");
            return OK;
        }
        else {
            ERROR("accept");
        }
        return ERR_ACCEPT;
    }

    err = enable_nonblocking(fd);
    if (err) {
        ERROR("Cannot enable nonblocking mode for fd: %d", fd);
        return err;
    }

    if (fd >= MAXFDS) {
        ERROR("fd (%d) >= MAXFDS (%d)", fd, MAXFDS);
        return ERR_MAXFD;
    }
    
    /* Create and allocate new client structure. */
    struct client *c = malloc(sizeof(struct client));
    c->fd = fd;
    c->len = 0;
    
    /* Initialize the http parser for this connection. */
    http_parser_init(&c->hp, HTTP_REQUEST);
    c->hp.data = c;
    
    /* Initialize ringbuffer. */
    rb_init(&c->resprb, c->buff, RESPRB_SIZE);

    DEBUG("New connection: %d", fd);
    err = WANT_READ(c, true);
    if (err) {
        ERROR("Cannot register fd: %d for read", fd);
    }

    return OK;
}

void sigint(int s) {
    DEBUG("SIGINT");
    free(tmp);
    exit(EXIT_SUCCESS);
}


static int
loop(int listenfd) {
    int err;
    struct epoll_event ev;
  
    signal(SIGINT, sigint);

    /* Allocate memory for temp buffer. */
    tmp = malloc(TMPSIZE);

    epollfd = epoll_create1(0);
    if (epollfd < 0) {
        ERROR("Creating epoll");
        return epollfd;
    }

    /* Accept event */
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    err = epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);
    if (err < 0) {
        ERROR("epoll_ctl EPOLL_CTL_ADD");
        return err;
    }

    struct epoll_event *events = calloc(MAXFDS, sizeof(struct epoll_event));
    if (events == NULL) {
        ERROR("Unable to allocate memory for epoll_events");
        return ERR_MEM;
    }

    for (;;) {
        int nready = epoll_wait(epollfd, events, MAXFDS, -1);

        for (int i = 0; i < nready; i++) {
            if (events[i].events & EPOLLERR) {
                DEBUG("epoll_wait returned EPOLLERR");
            }

            if (events[i].data.fd == listenfd) {
                /* The listening socket is ready; 
                 * this means a new peer is connecting.
                 */
                err = newconnection(&events[i]);
                if (err) {
                    ERROR("Accepting new connection");
                    return err;
                }
            }
            else {
                /* A peer socket is ready. */
                err = io(events[i]);
                if (err) {
                    ERROR("Processing socket IO.");
                    return err;
                }
            }
        }
    }
    return OK;
}


int
httpd_fork(struct httpd *m) {
    int listenfd;
    int i;
    pid_t pid;

    /* Create and listen tcp socket */
    listenfd = tcp_listen(&(m->port));
    if (listenfd < 0) {
        ERROR("Listening socket.");
        return listenfd;
    }

    if (m->forks == 0) {
        ERROR("Invalid number of forks: %d", m->forks);
        return ERR_ARGS;
    }

    /* Allocate memory for children pids. */
    m->children = calloc(m->forks, sizeof(pid_t));
    if (m->children == NULL) {
        return ERR_MEM;
    }

    for (i = 0; i < m->forks; i++) {
        DEBUG("Forking: %d", i);
        pid = fork();
        if (pid == -1) {
            ERROR("Cannot fork");
            return ERR_FORK;
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

            return loop(listenfd);
        }
    }
    return OK;
}

void
httpd_terminate(struct httpd *m) {
    int i;
    for (i = 0; i < m->forks; i++) {
        kill(m->children[i], SIGINT);
    }
}

int
httpd_join(struct httpd *m) {
    int status;
    int ret;
    int i;

    for (i = 0; i < m->forks; i++) {
        waitpid(m->children[i], &status, 0);
        ret |= WEXITSTATUS(status);
    }

    free(m->children);
    return ret;
}
