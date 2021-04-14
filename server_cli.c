#include "common.h"
#include "httpd.h"
#include "logging.h"
#include "cli.h"

#include <stdio.h>
#include <argp.h>
#include <stdlib.h>

#define SERVER_DEFAULT_PORT     8080
#define SERVER_DEFAULT_FORKS    1

static struct {
    uint16_t port;
    uint8_t forks;
} settings = {
    SERVER_DEFAULT_PORT,
    SERVER_DEFAULT_FORKS,
};

static char doc[] = "HTTP echo server using Linux epoll.";
static char args_doc[] = "";

/* Options definition */
static struct argp_option options[] = {
    ARG_VERBOSITY,
    ARG_CONCURRENCY,
    { "port", 'p', "PORT", 0,
     "TCP Port to bind, default: " STR(SERVER_DEFAULT_PORT) "." },
    { 0 }
};

/* Parse a single option. */
static int
parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'c':
            settings.forks = atoi(arg);
            break;

        case 'p':
            settings.port = atoi(arg);
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

int
servercli_run(int argc, char **argv) {

    /* Parse CLI arguments. */
    argp_parse(&argp, argc, argv, 0, 0, NULL);

    /* Configure HTTP server */
    struct httpd server = {
        .forks = settings.forks,
        .port = settings.port
    };

    /* Start it */
    if (httpd_start(&server)) {
        ERROR("Cannot start http server");
        return ERR;
    }

    INFO("Listening on port: %d", server.port);

    if (ev_join((struct ev *) &server)) {
        ERROR("Cannot kill chilren!");
        return ERR;
    }
    return OK;
}
