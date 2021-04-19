#ifndef FIXTURE_PCAPT_H
#define FIXTURE_PCAPT_H

#define CAPTMAX 1024

typedef int (*pfunc_t)(int argc, char **argv);

struct pcapt {
    pid_t child;
    const char *prog;
    char out[CAPTMAX + 1];
    char err[CAPTMAX + 1];
    int outpipe[2];
    int errpipe[2];
};

int pcaptw(struct pcapt *p, pfunc_t f, int timeout, int argc, char **argv);
int pcapt(struct pcapt *p, pfunc_t f, int argc, char **argv);
int pcapt_join(struct pcapt *p);
void pcapt_kill(struct pcapt *p);

#define NARGS(t, ...)  (sizeof((t[]){ __VA_ARGS__})/sizeof(t))
#define NCARGS(...)  NARGS(char *, __VA_ARGS__)

#define PCAPT(p_, f, ...) \
    pcapt(p_, f, NCARGS( __VA_ARGS__), (char *[]){ __VA_ARGS__})


#define PCAPTW(p_, f, t, ...) \
    pcaptw(p_, f, t, NCARGS( __VA_ARGS__), (char *[]){ __VA_ARGS__})

#define PCAPTW0(p_, f, ...) PCAPTW(p_, f, 0, __VA_ARGS__ ) 
#define PCAPT_KILL(p_) ({pcapt_kill(p_); pcapt_join(p_);})
#endif
