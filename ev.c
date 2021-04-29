/* local */
#include "logging.h"
#include "helpers.h"
#include "ev.h"
#include "ev_epoll.h"

/* system */
#include <unistd.h>

int
ev_server_start(struct evs *evs) {

    /* Create and listen tcp socket */
    evs->listenfd = tcp_listen(&(evs->bind));
    if (evs->listenfd == ERR) {
        ERROR("Cannot bind on: %d", evs->bind);
        return ERR;
    }
    
    /* Initialize event loop. */
    if (ev_epoll_server_init(evs)) {
        return ERR;
    }

    /* Fork and start multiple instance of server. */
    if(ev_common_fork(evs, (ev_cb_t) ev_epoll_server_loop)) {
        /* loop error */
        ev_epoll_server_deinit(evs);
        return ERR;
    }

    /* loop ok */
    return OK;
}

int
ev_server_terminate(struct evs *evs) {
    int ret = ev_common_terminate((struct ev *) evs);
    ev_epoll_server_deinit(evs);
    close(evs->listenfd);
    return ret;
}

int
ev_server_join(struct evs *evs) {
    int ret = ev_common_join((struct ev *) evs);
    ev_epoll_server_deinit(evs);
    close(evs->listenfd);
    return ret;
}
