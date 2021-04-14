#ifndef ev_h
#define ev_h

#include "options.h"
#include "ringbuffer.h"
#include <sys/types.h>
#include <sys/epoll.h>
#include <inttypes.h>

struct ev {
    uint8_t forks;
    pid_t *children;
    void *on_recvd;
    void *on_writefinish;
};

struct peer {
    int fd;
    char buff[EV_WRITE_BUFFSIZE];
    size_t len;
    // TODO: rename to wrb
    struct ringbuffer resprb;
    void *handler;
};

typedef int (*ev_cb_t)(struct ev * ev, struct peer * c);
typedef int (*ev_recvcb_t)(struct ev * ev, struct peer * c, const char *data,
                           size_t len);

struct evs {
    struct ev;
    int listenfd;
    uint16_t port;
    ev_cb_t on_connect;
};

/* Common */
void ev_terminate(struct ev *m);
int ev_join(struct ev *);
void ev_ctl(struct peer *c, int op, uint32_t e);
void ev_del_close(struct peer *c);

/* Server */
void evs_fork(struct evs *);

/* Event control macros. */
#define EV_ADD_READ(c) ev_ctl((c), EPOLL_CTL_ADD, EPOLLIN)
#define EV_MOD_READ(c) ev_ctl((c), EPOLL_CTL_MOD, EPOLLIN)
#define EV_MOD_WRITE(c) ev_ctl((c), EPOLL_CTL_MOD, EPOLLOUT)

#endif
