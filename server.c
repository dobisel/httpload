#include <logging.h>
#include <server.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <netinet/in.h>


#define BUFFSIZE          1024
#define MAXFDS              32
#define BACKLOG             16
#define OK                   0
#define ERR_LISTEN          -2
#define ERR_ENNONBLOCK      -3
#define ERR_MEM             -4
#define ERR_ACCEPT          -5
#define ERR_MAXFD           -6
#define ERR_FORK            -7
#define ERR_ARGS            -8


struct client {
    int fd;
    char buff[BUFFSIZE];
    uint16_t len;
    bool alive;
};


// TODO: EPOLLRDHUP


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
tcp_listen(uint16_t *port) {
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
want_close(int epollfd, struct client *c) {
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
want_read(int epollfd, struct client *c, bool add) {
    struct epoll_event ev = {0};
    ev.data.ptr = c;
    ev.events = EPOLLIN | EPOLLONESHOT;
    return epoll_ctl(epollfd, add? EPOLL_CTL_ADD: EPOLL_CTL_MOD, c->fd, &ev);
}


static int
want_write(int epollfd, struct client *c) {
    struct epoll_event ev = {0};
    ev.data.ptr = c;
    ev.events = EPOLLOUT | EPOLLONESHOT;
    return epoll_ctl(epollfd, EPOLL_CTL_MOD, c->fd, &ev);
}


static int 
newconnection(int epollfd, struct epoll_event *event) {
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
        ERROR("Cannot enable nonblocking mode for fd: %d",
              fd);
        return err;
    }

    if (fd >= MAXFDS) {
        ERROR("fd (%d) >= MAXFDS (%d)", fd, MAXFDS);
        return ERR_MAXFD;
    }

    struct client *c = malloc(sizeof(struct client));
    c->fd = fd;
    c->alive = true;
    c->len= 0;

    DEBUG("New connection: %d", fd);
    err = want_read(epollfd, c, true);
    if (err) {
        ERROR("Cannot register fd: %d for read", fd);
    }
    
    return OK;
}


static int
io(int epollfd, struct epoll_event ev) {
    int err;
    struct client *c = (struct client*) ev.data.ptr;
    if (ev.events & EPOLLRDHUP) {
        /* Closed */
        err = want_close(epollfd, c);
        if (err) {
            ERROR("Cannot close: %d", c->fd);
        }
    }
    else if (ev.events & EPOLLIN) {
        /* Read */
        c->len = read(c->fd, c->buff, BUFFSIZE);
        if (c->len) {
            err = want_write(epollfd, c);
            if (err) {
                ERROR("Cannot register fd: %d for write", c->fd);
            }
        }
        else {
            err = want_close(epollfd, c);
            if (err) {
                ERROR("Cannot close: %d", c->fd);
            }
        }
    }
    else if (ev.events & EPOLLOUT) {
        /* Write */
        write(c->fd, c->buff, c->len);
        c->len = 0;
        err = want_read(epollfd, c, false);
        if (err) {
            ERROR("Cannot register fd: %d for read", c->fd);
        }
    }
    return OK;
}


static int
loop(int listenfd) {
    int err;
    struct epoll_event ev;

    int epollfd = epoll_create1(0);
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
                err = newconnection(epollfd, &events[i]);
                if (err) {
                    ERROR("Accepting new connection");
                    return err;
                }
            }
            else {
                /* A peer socket is ready. */
                err = io(epollfd, events[i]);
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
        if (pid > 0 ) {
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


int
httpd_join(struct httpd *m) {
    int status;
    int ret; 
    int i;

    for (i = 0; i < m->forks; i++) {
        kill(m->children[i], SIGKILL);
        wait(&status);
        ret |= WEXITSTATUS(status);
    }
    
    free(m->children);
    return ret;
}
