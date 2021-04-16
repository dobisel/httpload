#ifndef EV_EPOLL_H
#define EV_EPOLL_H

#include "ev.h"

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

#define EV_ADD_READ_FD(fd) ev_ctlfd((fd), EPOLL_CTL_ADD, EPOLLIN)
#define EV_MOD_READ_FD(fd) ev_ctlfd((fd), EPOLL_CTL_MOD, EPOLLIN)
#define EV_MOD_WRITE_FD(fd) ev_ctlfd((fd), EPOLL_CTL_MOD, EPOLLOUT)

#endif
