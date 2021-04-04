#ifndef LOGGING_H
#define LOGGING_H

#include "common.h"


#include <stdio.h>
#include <errno.h>
#include <string.h>


enum log_level{
    LOG_ERROR,        // 0
    LOG_WARNING,    // 1
    LOG_INFO,        // 2
    LOG_DEBUG,        // 3
};


extern char log_level;

extern const char * log_levelnames[];


#define LOG_FP stdout
#define LOG_SHOULD_I( level ) ((level) <= log_level)


#define ERR(fmt, ...) \
    fprintf(LOG_FP, "[%s] %s -- " fmt CR, \
            log_levelnames[LOG_ERROR], \
            errno? strerror(errno): "(errno == 0)", \
            ## __VA_ARGS__ ); \
    fflush(LOG_FP); 


#define DBG(fmt, ...) \
    if (LOG_SHOULD_I(LOG_DEBUG)) { \
        fprintf(LOG_FP, "[%s] [%s:%d] " fmt CR, log_levelnames[LOG_DEBUG], \
                __FUNCTION__, __LINE__, ## __VA_ARGS__); \
        fflush(LOG_FP); \
    }


#define LOG(level, fmt, ...) \
    if (LOG_SHOULD_I(level)) { \
        fprintf(LOG_FP, "[%s] " fmt CR, log_levelnames[(level)], \
                ## __VA_ARGS__); \
        fflush(LOG_FP); \
    }


#define WRN( ... ) LOG(LOG_WARNING, __VA_ARGS__ )
#define INF( ... ) LOG(LOG_INFO, __VA_ARGS__ )


void log_init(enum log_level level);


#endif
