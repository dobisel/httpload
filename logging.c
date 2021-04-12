#include "logging.h"

char log_level = LL_INFO;

const char *log_levelnames[] = {
    "Unknown",                  // 0
    "Error",                    // 1
    "Warning",                  // 2
    "Info",                     // 3
    "Debug",                    // 4
};

void
log_setlevel(loglevel_t level) {
    log_level = level;
}
