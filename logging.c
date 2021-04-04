#include "logging.h"


char log_level = LOG_INFO;


const char * log_levelnames [] = {
    "Error",   // 0
    "Warning", // 1
    "Info",    // 2
    "Debug",   // 3
};


void log_init(enum log_level level) {
    log_level = level;
    INFO("Logging Initialized, level: %s", log_levelnames[level]);
}

