#ifndef FIXTURE_STDCAPT_H
#define FIXTURE_STDCAPT_H

#include <stdbool.h>

#define STDCAPTMAX  1024

enum captopt {
    STDCAPT_BOTH,
    STDCAPT_NO_OUT,
    STDCAPT_NO_ERR,
};

struct captfile {
    char name[8];
    int fd;
    int backfd;
};

struct capt {
    struct captfile stdout;
    struct captfile stderr;
    bool running;
    enum captopt options;
    
    char out[STDCAPTMAX + 1];
    char err[STDCAPTMAX + 1];
};

void capt_start(struct capt *c, enum captopt options);
void capt_restore(struct capt *c);

#define STDCAPT(c)     capt_start(&(c), STDCAPT_NO_OUT);
#define STDCAPT_ERR(c) capt_start(&(c), STDCAPT_NO_OUT);
#define STDCAPT_OUT(c) capt_start(&(c), STDCAPT_NO_ERR);
#define EQOUT(c, e)    capt_restore(&(c));EQS(c.out, (e))
#define EQERR(c, e)    capt_restore(&(c));EQS(c.err, (e))

#endif
