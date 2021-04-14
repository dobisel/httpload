#ifndef LOGGING_H
#define LOGGING_H

#include "common.h"
#include "cli.h"

#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

enum loglevel {
    LL_UNKNOWN,
    LL_ERROR,
    LL_WARN,
    LL_INFO,
    LL_DEBUG,
};

#define LOG_LEVEL_ISVALID(l) ((l <= LL_DEBUG) && (l >= LL_ERROR))

extern char log_level;
extern const char *log_levelnames[];

#define LOG_OK( level ) ((level) <= log_level)
#define ERRX( ... ) err(EXIT_FAILURE, __VA_ARGS__);
#define WARN( ... ) if LOG_OK(LL_WARN) warn( __VA_ARGS__ )
#define INFO(fmt, ... ) if LOG_OK(LL_INFO) printf(fmt N, ## __VA_ARGS__ )
#define DBUG(fmt, ...) if LOG_OK(LL_DEBUG) \
    printf("%03d:%s " fmt N, __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define CHK( ... ) DBUG( __VA_ARGS__ )

typedef uint8_t loglevel_t;

void log_setlevel(loglevel_t level);

#endif
