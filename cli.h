#ifndef CLI_H
#define CLI_H

#include "logging.h"

#include <argp.h>

/* Common GNU argp arguments */
#define ARG_VERBOSITY { "verbosity", 'v', "LEVEL", 0, "Verbosity levels: " \
     STR(LL_ERROR) ": Error, " \
     STR(LL_WARN)  ": Warning, " \
     STR(LL_INFO)  ": Info, " \
     STR(LL_DEBUG) ": Debug. " \
     "default: " STR(LL_INFO) "." }

#define ARG_CONCURRENCY { "concurrency", 'c', "1..N", 0, \
    "Number of multiple requests to make at a time (number of processes). " \
    "It's recommented to use (CPU_COUNT - 1). " \
    "default: " STR(SERVER_DEFAULT_FORKS) "." }

int 
verbosity_parse(struct argp_state *state, const char *val);

#endif
