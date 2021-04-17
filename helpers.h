#ifndef HELPERS_H
#define HELPERS_H

#include <inttypes.h>
#include <netinet/in.h>

#define MIN(x, y) (((x) > (y))? (y): (x))
#define MAX(x, y) (((x) < (y))? (y): (x))

/* Check here: https://stackoverflow.com/a/5459929/680372 */
#define STR_(x) #x
#define STR(x) STR_(x)

int enable_nonblocking(int fd);
int tcp_listen(uint16_t * port);

#endif
