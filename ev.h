#ifndef ev_h
#define ev_h

#include "ev_common.h"

void ev_server_start(struct evs *evs);
void ev_server_terminate(struct evs *evs);
int ev_server_join(struct evs *evs);

#endif
