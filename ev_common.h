#ifndef ev_common_h
#define ev_common_h

/*

                           +---------------+           +------------------+
                           | struct peer   |   +------>| enum peer_state  |
                           +---------------+   |       +------------------+
                           |  fd           |   |       |  PS_READ         |
                           |  state        >---+       |  PS_WRITE        |
                           |  writebuff[]  |           +------------------+
                           |  writerb      |
                           | *handler      |           +------------------+
                           +---------------+       +-->| struct ev_epoll  |
                                                   |   +------------------+
  +-----------------+      +---------------+       |   |  epollfd         |
  | struct ev       |  +-->| union ev_priv |       |   +------------------+
  +-----------------+  |   +---------------+       |
  |  id             |  |   |  ev_epoll     >-------+   +------------------+
  |  forks          |  |   |  ev_select    >---------->| struct ev_select |
  |  children[]     |  |   |  ev_mock      >-------+   +------------------+
  |  private_data   >--+   +---------------+       |   | ?                |
  +-----------------+                              |   +----------------- +
  | *on_recvd       |                              |
  | *on_writefinish |                              |   +------------------+
  +-----------------+                              +-->| struct ev_mock   |
      ^         ^                                      +------------------+
      |         |                                      | ?                |
      |         +--------------+                       +------------------+
      |                        |
  +---^-----------+        +---^------------+
  | struct ev_srv |        | struct ev_clnt |
  +---------------+        +----------------+
  | +struct ev    |        | +struct ev     |
  |  listenfd     |        |  hostname      |
  |  bind         |        |  port          |
  +---------------+        +----------------+
  | *on_connect   |        |  ?             |
  +---------------+        +----------------+

*/

#include "options.h"
#include "ringbuffer.h"
#include <sys/types.h>
#include <sys/epoll.h>
#include <inttypes.h>
#include <stdbool.h>

struct ev_epoll;

union ev_priv {
    struct ev_epoll *epoll;
};

enum peer_state {
    PS_UNKNOWN,
    PS_CLOSE,
    PS_READ,
    PS_WRITE
};

struct ev;
struct evs;

struct peer {
    int fd;
    enum peer_state status;
    char writebuff[EV_WRITE_BUFFSIZE];
    struct ringbuffer writerb;
    void *handler;
};

typedef void (*evs_conncb_t)(struct evs * evs, struct peer * c);
typedef void (*ev_writecb_t)(struct ev * ev, struct peer * c);
typedef void (*ev_recvcb_t)(struct ev * ev, struct peer * c, const char *data,
                            size_t len);
struct ev {
    uint8_t id;
    uint8_t forks;
    bool cancel;
    pid_t *children;
    union ev_priv;
    ev_recvcb_t on_recvd;
    ev_writecb_t on_writefinish;
};

struct evs {
    struct ev;
    int listenfd;
    uint16_t bind;
    evs_conncb_t on_connect;
    evs_conncb_t on_disconnect;
};

typedef int (*ev_cb_t)(struct ev * ev);

struct peer *ev_common_newconn(struct evs *evs);
void ev_common_peer_disconn(struct evs *evs, struct peer *c);
void ev_common_read(struct ev *ev, struct peer *c);
void ev_common_write(struct ev *ev, struct peer *c);
int ev_common_terminate(struct ev *ev);
int ev_common_join(struct ev *ev);
int ev_common_fork(struct ev *ev, ev_cb_t loop);
void ev_common_init(struct ev *ev);
void ev_common_deinit(struct ev *ev);

#endif
