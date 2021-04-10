#ifndef LOGGING_H
#define LOGGING_H

#include "common.h"
#include "cli.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>


#define LOG_ERROR   0
#define LOG_WARN    1
#define LOG_INFO    2
#define LOG_DEBUG   3


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
        fprintf(stderr, " -- errno: %d additional info: %s" CR, \
                errno, strerror(errno)); \
    else fprintf(stderr, CR); \
    fflush(LOG_FP); \
    exit(-1);


#define ERROR(fmt, ...) \
    fprintf(stderr, fmt, ## __VA_ARGS__ ); \
    if (errno) \
        fprintf(stderr, " -- errno: %d additional info: %s" CR, \
                errno, strerror(errno)); \
    else fprintf(stderr, CR); \
    fflush(LOG_FP);


#define DEBUG(fmt, ...) \
    if (LOG_SHOULD_I(LOG_DEBUG)) { \
        fprintf(LOG_FP, LOG_LEVEL_FMT "[%s:%d] " fmt CR, \
                log_levelnames[LOG_DEBUG], \
                __FUNCTION__, __LINE__, ## __VA_ARGS__); \
        fflush(LOG_FP); \
    }


#define LOG(level, fmt, ...) \
    if (LOG_SHOULD_I(level)) { \
        fprintf(LOG_FP, LOG_LEVEL_FMT fmt CR, \
                log_levelnames[(level)], \
                ## __VA_ARGS__); \
        fflush(LOG_FP); \
    }


#define WARN( ... ) LOG(LOG_WARN, __VA_ARGS__ )
#define INFO( ... ) LOG(LOG_INFO, __VA_ARGS__ )


typedef uint8_t loglevel_t;

void log_setlevel(loglevel_t level);


#endif
