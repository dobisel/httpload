#include "common.h"
#include "logging.h"
#include "cli.h"

#include <stdio.h>
#include <argp.h>
#include <stdlib.h>

static struct {
    char *verb;
    char *url;
} _settings = {
    "GET",
};

static char _doc[] = "HTTP stress test using Linux epoll.";
static char _args_doc[] = "URL [VERB]";

/* Options definition */
static struct argp_option _options[] = {
    ARG_VERBOSITY,
    ARG_CONCURRENCY,
    { 0 }
};

/* Parse a single option. */
static int
_parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case ARGP_KEY_ARG:
            if (state->arg_num == 0) {
                _settings.url = arg;
            }
            else if (state->arg_num == 1) {
                _settings.verb = arg;
            }
            else if (state->arg_num >= 2) {
                /* Too many arguments. */
                return ARGP_ERR_UNKNOWN;
            }
            break;

        default:
            return parse_common_opts(key, arg, state);
    }
    return EXIT_SUCCESS;
}

static struct argp _argp = { _options, _parse_opt, _args_doc, _doc };

int
clientcli_run(int argc, char **argv) {
    argp_parse(&_argp, argc, argv, 0, 0, NULL);

    /* Do the job */
    return EXIT_SUCCESS;
}
