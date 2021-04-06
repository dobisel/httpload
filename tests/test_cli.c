#include "cli.h"
#include "testing.h"
#include "logging.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


static int stdoutback;
static fpos_t stdoutposback;


void switchstdout(const char *newfile) {
    fflush(stdout);
    fgetpos(stdout, &stdoutposback);
    errno = 0;
    clearerr(stdout);
    stdoutback = dup(fileno(stdout));
    freopen(newfile, "w", stdout);
}


void restorestdout() {
    fflush(stdout);
    dup2(stdoutback, fileno(stdout));
    close(stdoutback);
    clearerr(stdout);
    fsetpos(stdout, &stdoutposback);
}


int capture(int argc, char **argv, char *const out, int outlen) {
    char *stdoutfile = "stdout.tmp";
    switchstdout(stdoutfile);
    ERROR("RUN");
    int ret = cli_run(argc, argv);
    restorestdout();
    FILE *of = fopen(stdoutfile, "r");
    fgets(out, outlen, of);
    fclose(of);
    return ret;
}

#define PROG    "httpload"


void test_version() {
    char out[1024] = {0};
    char *p[2] = {PROG, "--version"};
    int status = capture(2, p, out, 1024);
    
    eqint(status, 0);
    //eqstr("[I] Logging Initialized, level: Info" CR, out);
}


//void test_verbosity() {
//    char out[1024] = {0};
//    char *p[] = {PROG, "-v", "0"};
//    int status = capture(3, p, out, 1024);
//    eqint(status, 0);
//    eqstr(CR, out);
//}


int main() {
    test_version();
    //test_verbosity();
}

