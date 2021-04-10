#include "logging.h"
#include "capture.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


#define STDOUT  1
#define STDERR  2


static int
execchild(mainfunc_t f, const char *prog, int argc, char **argv) {
    char **newargv = malloc(sizeof(char *) * (argc + 1));
    newargv[0] = (char *) prog;
    memcpy(newargv + 1, argv, argc * sizeof(char *));
    argc++;

    int status = f(argc, newargv);
    free(newargv);
    return status;
}


int
fcapture(mainfunc_t f, const char *prog, int argc, char **argv,
        char *const out, char *const err) {
    int outpipe[2];
    int errpipe[2];
    pid_t pid;
    int status = EXIT_SUCCESS;
    ssize_t bytes;

    /* Trim output and err. */
    out[0] = err[0] = 0;

    /* Flush stdout before fork. */
    fflush(stdout);

    /* Prepare stdout pipe. */
    if (pipe(outpipe)) {
        ERROR("stdout pipe");
        return EXIT_FAILURE;
    }

    /* Prepare stderr pipe. */
    if (pipe(errpipe)) {
        ERROR("stderr pipe");
        return EXIT_FAILURE;
    }

    /* Fork */
    pid = fork();
    if (pid == -1) {
        ERROR("fork");
        return EXIT_FAILURE;
    }
    else if (pid > 0) {
        /* Parent */

        /* Wait for child process to terminate. */
        wait(&status);

        /* Close write side of output pipe. */
        close(outpipe[1]);
        close(errpipe[1]);

        /* Read stdout then close. */
        bytes = read(outpipe[0], out, CAPTMAX);
        out[bytes] = 0;
        close(outpipe[0]);

        /* Read stderr then close. */
        bytes = read(errpipe[0], err, CAPTMAX);
        err[bytes] = 0;
        close(errpipe[0]);
        status = WEXITSTATUS(status);
    }
    else if (pid == 0) {
        /* Child */

        /* Close the read side of stdout and stderr. */
        close(outpipe[0]);
        close(errpipe[0]);

        /* Redirect stdout and stderr. */
        dup2(outpipe[1], STDOUT);
        dup2(errpipe[1], STDERR);

        /* Run cli main & exit */
        status = execchild(f, prog, argc, argv);
        close(outpipe[1]);
        close(errpipe[1]);
        exit(status);
    }
    return status;
}
