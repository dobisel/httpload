#include "common.h"
#include "logging.h"
#include "cli.h"

#include <argp.h>

const char *argp_program_version = HTTPLOAD_VERSION;
const char *argp_program_bug_address = HTTPLOAD_URL;

int
verbosity_parse(struct argp_state *state, const char *val) {
    int v = atoi(val);

    if (!LOG_LEVEL_ISVALID(v)) {
        argp_error(state, "Invalid verbosity level: %d.", v);
    }
    return v;
}

int
parse_common_opts(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'v':
            log_setlevel(verbosity_parse(state, arg));
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return OK;
}
