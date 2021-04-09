#include "common.h"
#include "client.h"
#include "logging.h"

#include <stdio.h>
#include <argp.h>
#include <stdlib.h>


static struct {
    enum log_level verbosity;
    char *verb;
    char *url;
} settings = {
    LOG_INFO,
    "GET",
};


const char *argp_program_version = HTTPLOAD_VERSION;
const char *argp_program_bug_address = "http://github.com/dobisel/httpload";
static char doc[] = "HTTP stress test using Linux epoll.";
static char args_doc[] = "URL [VERB]";


/* Options definition */
static struct argp_option options[] = {
    {
     "verbosity", 'v', "LEVEL", 0,
     "Verbosity level: 0: Error, 1: Warning, 2: Info, 3: Debug. default: 2"},
    {0}
};


/* Parse a single option. */
static int
parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
    case 'v':
        settings.verbosity = atoi(arg);
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num == 0) {
            settings.url = arg;
        }
        else if (state->arg_num == 1) {
            settings.verb = arg;
        }
        else if (state->arg_num >= 2) {
            /* Too many arguments. */
            ERROR("Too many arguments");
            argp_usage(state);
            return ARGP_ERR_UNKNOWN;
        }
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return EXIT_SUCCESS;
}


static struct argp argp = { options, parse_opt, args_doc, doc };


int
clientcli_run(int argc, char **argv) {
    int err = argp_parse(&argp, argc, argv, ARGP_NO_EXIT, 0, NULL);
    if (err) {
        return err;
    }

    log_setlevel(settings.verbosity);
    return EXIT_SUCCESS;
}
