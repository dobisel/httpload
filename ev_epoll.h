#ifndef EV_EPOLL_H
#define EV_EPOLL_H

#include "ev_common.h"

/* Server */
int ev_epoll_server_init(struct evs *evs);
void ev_epoll_server_deinit(struct evs *evs);
int ev_epoll_server_loop(struct evs *evs);

#endif
