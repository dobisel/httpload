#ifndef FIXTURE_CAPTURE_H
#define FIXTURE_CAPTURE_H

#define CAPTMAX 1024

typedef int (*mainfunc_t)(int argc, char **argv);


int
fcapture(mainfunc_t f, const char *prog, int argc, char **argv,
        char *const out, char *const err);

#endif
