#ifndef CLI_H
#define CLI_H

#include "logging.h"

/* Common GNU argp arguments */
#define ARG_VERBOSITY { "verbosity", 'v', "LEVEL", 0, "Verbosity levels: " \
     STR(LOG_ERROR) ": Error, " \
     STR(1) ": Warning, " \
     STR(2) ": Info, " \
     STR(3) ": Debug. " \
     "default: " STR(2) "." }

#define ARG_CONCURRENCY { "concurrency", 'c', "1..N", 0, \
    "Number of multiple requests to make at a time (number of processes). " \
    "It's recommented to use (CPU_COUNT - 1). " \
    "default: " STR(SERVER_DEFAULT_FORKS) "." }

#endif
