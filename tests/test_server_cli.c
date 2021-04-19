#include "logging.h"
#include "server_cli.h"
#include "fixtures/assert.h"
#include "fixtures/pcapt.h"
#include "fixtures/curl.h"
#include <unistd.h>

static struct pcapt p = {.prog = "httploads"};

#define SCAPTW0(...)    PCAPTW0   (&p, servercli_run, __VA_ARGS__)
#define SCAPT(...)      PCAPT     (&p, servercli_run, __VA_ARGS__)
#define SCAPT_KILL()    PCAPT_KILL(&p)

static void
test_version(struct test *t) {
    EQI(SCAPTW0("--version"), 0);
    EQS(p.out, HTTPLOAD_VERSION N);
    EQS(p.err, "");

    EQI(SCAPTW0("-V"), 0);
    EQS(p.out, HTTPLOAD_VERSION N);
    EQS(p.err, "");
}

static void
test_fork(struct test *t) {
    EQI(SCAPT("-c2"), 0);
    EQS(p.err, "");
    EQS(p.out, "");
    EQI(HTTPGET("http://localhost:8080/"), 200);
    SCAPT_KILL();
}

static void
test_invalidargument(struct test *t) {
    /* Invalid optional argument. */
    NEQI(SCAPTW0("--dry", "--invalidargument", "0"), 0);
    EQS(p.out, "");
    EQNS(49, p.err, "httploads: unrecognized option '--invalidargument'");

    /* Extra positional arguments. */
    NEQI(SCAPTW0("--dry", "foo", "bar", "baz"), 0);
    EQS(p.out, "");
    EQNS(29, p.err, "httploads: Too many arguments");

    /* Invalid fork counts. */
    NEQI(SCAPTW0("--dry", "-c0"), 0);
    EQS(p.out, "");
    EQNS(42, p.err, "test_server_cli: Invalid number of forks: 0");
}

static void
test_bind(struct test *t) {
    EQI(SCAPT("-b4444"), 0);
    EQI(HTTPGET("http://localhost:4444/"), 200);
    SCAPT_KILL();
    EQS(p.err, "");
    EQS(p.out, "Listening on port: 4444" N);
    
    /* Listen on port < 1024 */
    EQI(SCAPTW0("-b888"), 1);
    EQS(p.out, "");
    EQS(p.err, "test_server_cli: Cannot bind on: 30723: Permission denied" N);
    SCAPT_KILL();
}

static void
test_dryrun(struct test *t) {
    EQI(SCAPTW0("--dry"), 0);
    EQS(p.err, "");
    EQS(p.out, 
        "forks:\t\t1" N 
        "bind:\t\t8080" N
        "verbosity:\tDebug(4)" N);
}

int
main() {
    struct test t;

    log_setlevel(LL_DEBUG);
    SETUP(&t);
    test_version(&t);
    test_fork(&t);
    test_invalidargument(&t);
    test_bind(&t);
    test_dryrun(&t);
    return TEARDOWN(&t);
}
