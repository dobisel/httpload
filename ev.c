#include "logging.h"
#include "ev.h"
#include "ev_epoll.h"

void
ev_server_start(struct evs *evs) {
    ev_epoll_server_start(evs);
}

void
ev_server_terminate(struct evs *evs) {
    ev_common_terminate((struct ev *)evs);
}

int
ev_server_join(struct evs *evs) {
    return ev_epoll_server_join(evs);
}
