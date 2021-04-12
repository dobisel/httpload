#ifndef FIXTURE_CAPTURE_H
#define FIXTURE_CAPTURE_H

#define CAPTMAX 1024

typedef int (*mainfunc_t)(int argc, char **argv);


int
fcapture_timeout(int timeout, mainfunc_t f, const char *prog, int argc, 
        char **argv, char *const out, char *const err);

#define fcapture(...) fcapture_timeout(0, __VA_ARGS__);

#endif
