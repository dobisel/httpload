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

typedef uint8_t loglevel_t;
extern loglevel_t log_level;
extern const char *log_levelnames[];

#define LOG_OK(level) ((level) <= log_level)

#define ERROR( ... ) warn(__VA_ARGS__)
#define ERRORX( ... ) err(EXIT_FAILURE, __VA_ARGS__)
#define WARN( ... ) if LOG_OK(LL_WARN) warn( __VA_ARGS__ )
#define INFO(fmt, ... ) if LOG_OK(LL_INFO) printf(fmt N, ## __VA_ARGS__ )
#define DEBUG(fmt, ...) if LOG_OK(LL_DEBUG) \
    printf("%03d:%s -- " fmt N, __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define CHK( ... ) DEBUG( __VA_ARGS__ )

#define ERRORN( ... )  fprintf(stderr, ## __VA_ARGS__ )
#define WARNN( ... ) if LOG_OK(LL_WARN)  fprintf(stderr, ## __VA_ARGS__ )
#define INFON(fmt, ... ) if LOG_OK(LL_INFO) printf(fmt, ## __VA_ARGS__ )

#define INFOC(c) if LOG_OK(LL_INFO) putchr(c)

void log_setlevel(loglevel_t level);

#endif
