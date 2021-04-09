#ifndef FIXTUE_CLI_H
#define FIXTUE_CLI_H

#define CAPTMAX 1024

typedef int (*mainfunc_t)(int argc, char **argv);


int




cli_capture(mainfunc_t f, const char *prog, int argc, char **argv,
        char *const out, char *const err);
#endif
