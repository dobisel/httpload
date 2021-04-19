#include "logging.h"
#include "fixtures/pcapt.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


#define STDOUT  1
#define STDERR  2


static int
execchild(pfunc_t f, const char *prog, int argc, char **argv) {
    int status;
    char **newargv = malloc(sizeof(char *) * (argc + 1));
    newargv[0] = (char *) prog;
    if (argc) {
        memcpy(newargv + 1, argv, argc * sizeof(char *));
    }

    argc++;
    status = f(argc, newargv);
    free(newargv);
    return status;
}

int
pcapt(struct pcapt *p, pfunc_t f, int argc, char **argv) {
    pid_t pid;
    int status = EXIT_SUCCESS;

    if (p->out) {
        /* Trim output buffer. */
        p->out[0] = 0;

        /* Prepare stdout pipe. */
        if (pipe(p->outpipe)) {
            ERRX("stdout pipe");
        }
    }
    if (p->err) {
        /* Trim err buffer. */
        p->err[0] = 0;

        /* Prepare stderr pipe. */
        if (pipe(p->errpipe)) {
            ERRX("stderr pipe");
        }
    }

    /* Flush stdout and stderr before fork. */
    fflush(stdout);
    fflush(stderr);

    /* Fork */
    pid = fork();
    if (pid == -1) {
        ERRX("Cannot fork");
    }

    if (pid > 0) {
        /* Parent */
        p->child = pid;        
        return OK;
    }

    if (pid != 0) {
        /* fork() returned Something is wrong here! */
        ERRX("Cannot fork");
    }

    /* Child */
    if (p->out) {
        /* Close the read side of stdout. */
        close(p->outpipe[0]);

        /* Redirect stdout. */
        dup2(p->outpipe[1], STDOUT);
    }

    if (p->err) {
        /* Close the read side of stderr. */
        close(p->errpipe[0]);

        /* Redirect stderr. */
        dup2(p->errpipe[1], STDERR);
    }

    /* Run cli main & exit */
    status = execchild(f, p->prog, argc, argv);

    if (p->out) {
        /* Close the write side of stdout. */
        close(p->outpipe[1]);
    }

    if (p->err) {
        /* Close the write side of stderr. */
        close(p->errpipe[1]);
    }
    exit(status);
}

int
pcapt_join(struct pcapt *p) {
    int status;
    ssize_t bytes;

    /* Wait for child process to terminate. */
    wait(&status);

    if (p->out) {
        /* Close the write side of output pipe. */
        close(p->outpipe[1]);

        /* Read stdout then close. */
        bytes = read(p->outpipe[0], p->out, CAPTMAX);
        p->out[bytes] = 0;
        close(p->outpipe[0]);
    }

    if (p->err) {
        /* Close the write side of err pipe. */
        close(p->errpipe[1]);

        /* Read stderr then close. */
        bytes = read(p->errpipe[0], p->err, CAPTMAX);
        p->err[bytes] = 0;
        close(p->errpipe[0]);
    }

    return WEXITSTATUS(status);
}

void
pcapt_kill(struct pcapt *p) {
    kill(p->child, SIGINT);
}

int
pcaptw(struct pcapt *p, pfunc_t f, int timeout, int argc, char **argv) {
    
    if(pcapt(p, f, argc, argv)) {
        return ERR;
    }

    /* Kill child after timeout, if given. */
    if (timeout) {
        sleep(timeout);
        pcapt_kill(p);
    }
    
    return pcapt_join(p);
}
