#include "logging.h"
#include "ev.h"
#include "ev_epoll.h"

void
ev_server_start(struct evs *evs) {
    ev_epoll_server_start(evs);
}

int
ev_server_terminate(struct evs *evs) {
    return ev_epoll_server_terminate(evs);
}

/** Cannot cover due the GCC will not gather info of fork() parents. */
// LCOV_EXCL_START
int
ev_server_join(struct evs *evs) {
    return ev_epoll_server_join(evs);
}
// LCOV_EXCL_END
