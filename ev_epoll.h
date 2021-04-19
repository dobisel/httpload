#ifndef EV_EPOLL_H
#define EV_EPOLL_H

#include "ev_common.h"

/* Server */
void ev_epoll_server_start(struct evs *evs);
int ev_epoll_server_terminate(struct evs *evs);
int ev_epoll_server_join(struct evs *evs);

#endif
