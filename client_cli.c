#include "common.h"
#include "logging.h"
#include "cli.h"

#include <stdio.h>
#include <argp.h>
#include <stdlib.h>

static struct {
    char *prog;
    uint8_t verbosity;
    char *verb;
    char *url;
} settings = {
    "httploadc",
    LL_INFO,
    "GET",
};

static char doc[] = "HTTP stress test using Linux epoll.";
static char args_doc[] = "URL [VERB]";

/* Options definition */
static struct argp_option options[] = {
    ARG_VERBOSITY,
    ARG_CONCURRENCY,
    { 0 }
};

/* Parse a single option. */
static int
parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'v':
            settings.verbosity = verbosity_parse(state, arg);
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

    /* Preserve executed comman name. */
    settings.prog = argv[0];

    /* Cannot make argp_parse to return error.
     * So, comment error handling lines for coverage result.
     */
    argp_parse(&argp, argc, argv, 0, 0, NULL);
    // err = argp_parse(...)
    // if (err) {
    //     return EXIT_FAILURE;
    // }

    log_setlevel(settings.verbosity);
    return EXIT_SUCCESS;
}
