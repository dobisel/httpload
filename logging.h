#ifndef LOGGING_H
#define LOGGING_H

#include "common.h"
#include "cli.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

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

#define LOG_ERR_FP stderr
#define LOG_FP stdout
#define LOG_SHOULD_I( level ) ((level) <= log_level)
#define LOG_LEVEL_FMT   "%s: "

#define FATAL(fmt, ...) \
    fprintf(stderr, "[%s:%d] fatal: " fmt, __FUNCTION__, __LINE__, \
        ## __VA_ARGS__ ); \
    if (errno) \
        fprintf(stderr, " -- errno: %d additional info: %s" N, \
                errno, strerror(errno)); \
    else fprintf(stderr, N); \
    fflush(LOG_FP); \
    exit(-1);

#define ERROR(fmt, ...) \
    fprintf(stderr, fmt, ## __VA_ARGS__ ); \
    if (errno) \
        fprintf(stderr, " -- errno: %d additional info: %s" N, \
                errno, strerror(errno)); \
    else fprintf(stderr, N); \
    fflush(LOG_FP);

#define DEBUG(fmt, ...) \
    if (LOG_SHOULD_I(LL_DEBUG)) { \
        fprintf(LOG_FP, LOG_LEVEL_FMT "[%s:%d] " fmt N, \
                log_levelnames[LL_DEBUG], \
                __FUNCTION__, __LINE__, ## __VA_ARGS__); \
        fflush(LOG_FP); \
    }

#define LOG(level, fmt, ...) \
    if (LOG_SHOULD_I(level)) { \
        fprintf(LOG_FP, LOG_LEVEL_FMT fmt N, \
                log_levelnames[(level)], \
                ## __VA_ARGS__); \
        fflush(LOG_FP); \
    }

#define WARN( ... ) LOG(LL_WARN, __VA_ARGS__ )
#define INFO( ... ) LOG(LL_INFO, __VA_ARGS__ )

typedef uint8_t loglevel_t;

void log_setlevel(loglevel_t level);

#endif
