#ifndef ev_h
#define ev_h

/*

                           +---------------+
                           | struct peer   |
                           +---------------+
                           |  fd           |
                           |  writebuff[]  |
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
  |  private_data   |--+   +---------------+       |   | ?                |
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

struct ev {
    uint8_t id;
    uint8_t forks;
    pid_t *children;
    void *on_recvd;
    void *on_writefinish;
};

struct peer {
    int fd;
    char writebuff[EV_WRITE_BUFFSIZE];
    struct ringbuffer writerb;
    void *handler;
};

typedef int (*ev_cb_t)(struct ev * ev, struct peer * c);
typedef int (*ev_recvcb_t)(struct ev * ev, struct peer * c, const char *data,
                           size_t len);

struct evs {
    struct ev;
    int listenfd;
    uint16_t bind;
    ev_cb_t on_connect;
};

#endif
