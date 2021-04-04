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
    char *stdoutfile = "build/stdout.txt";
    switchstdout(stdoutfile);
    int ret = cli_run(argc, argv);
    restorestdout();
    FILE *of = fopen(stdoutfile, "r");
    fgets(out, outlen, of);
    fclose(of);
    return ret;
}


void test_version() {
    char out[1024];
    char *p[] = {"-V"};
    int status = capture(1, p, out, 1024);
    eqint(status, 0);
    eqstr("[I] Logging Initialized, level: Info" CR, out);
}


int main() {
    test_version();
}

