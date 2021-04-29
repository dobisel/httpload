#include "common.h"
#include "httpd.h"
#include "logging.h" 
#include "cli.h"

#include <stdio.h>
#include <argp.h>
#include <stdlib.h>
#include <stdbool.h>

#define SERVER_DEFAULT_BIND     8080
#define SERVER_DEFAULT_FORKS    1

static struct {
    uint16_t bind;
    uint8_t forks;
    bool dryrun;
} settings = {
    SERVER_DEFAULT_BIND,
    SERVER_DEFAULT_FORKS,
    false,
};

static char doc[] = "HTTP echo server using Linux epoll.";
static char args_doc[] = "";

/* Options definition */
static struct argp_option options[] = {
    ARG_VERBOSITY,
    ARG_CONCURRENCY,
    ARG_DRYRUN,
    { "bind", 'b', "PORT", 0,
     "TCP Port to bind, default: " STR(SERVER_DEFAULT_BIND) "." },
    { 0 }
};

/* Parse a single option. */
static int
parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'c':
            settings.forks = atoi(arg);
            if (settings.forks <= 0) {
                ERRORX("Invalid number of forks: %d", settings.forks);
            }

            break;

        case 'b':
            settings.bind = atoi(arg);
            break;

        case ARG_DRYRUN_KEY:
            settings.dryrun = true;
            break;

        case ARGP_KEY_ARG:
            if (state->arg_num >= 0) {
                /* Too many arguments. */
                return ARGP_ERR_UNKNOWN;
            }
            break;

        default:
            return parse_common_opts(key, arg, state);
    }
    return EXIT_SUCCESS;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

static int
_dryrun() {
    INFO("forks:\t\t%d", settings.forks);
    INFO("bind:\t\t%d", settings.bind);
    INFO("verbosity:\t%s(%d)", log_levelnames[log_level], log_level);
    return EXIT_SUCCESS;
}

int
servercli_run(int argc, char **argv) {
    /* Set no buffer for stdout */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Parse CLI arguments. */
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    /* Configure HTTP server */
    struct httpd server = {
        .forks = settings.forks,
        .bind = settings.bind
    };

    /* Dry Run. */
    if (settings.dryrun) {
        return _dryrun();
    }

    /* Start it */
    if (httpd_start(&server)) {
        ERROR("Cannot start HTTP server.");
        return EXIT_FAILURE;
    }
   
    /* Prompt listening port. */
    INFO("Listening on port: %d", server.bind);
    
    /* Wait for server to stop. */
    int ret = httpd_join(&server);
    //__gcov_flush();
    return ret;
}
