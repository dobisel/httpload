#include "logging.h"


char log_level = LOG_INFO;


const char * log_levelnames [] = {
    "E",    // 0
    "W",    // 1
    "I",    // 2
    "D",    // 3
};


void log_init(enum log_level level) {
    log_level = level;
    INF("Logging Initialized, level: %s", log_levelnames[level]);
}

