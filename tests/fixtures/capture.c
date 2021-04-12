#include "logging.h"
#include "fixtures/capture.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


#define STDOUT  1
#define STDERR  2


static int
execchild(mainfunc_t f, const char *prog, int argc, char **argv) {
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
fcapture_timeout(int timeout, mainfunc_t f, const char *prog, int argc, 
        char **argv, char *const outbuff, char *const errbuff) {
    int outpipe[2];
    int errpipe[2];
    pid_t pid;
    int status = EXIT_SUCCESS;
    ssize_t bytes;

    /* Trim output and err. */
    outbuff[0] = errbuff[0] = 0;

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
        
        /* Kill child after timeout, if given. */
        if (timeout) {
            sleep(timeout);
            kill(pid, SIGINT);
        }

        /* Wait for child process to terminate. */
        wait(&status);

        /* Close write side of output pipe. */
        close(outpipe[1]);
        close(errpipe[1]);

        /* Read stdout then close. */
        bytes = read(outpipe[0], outbuff, CAPTMAX);
        outbuff[bytes] = 0;
        close(outpipe[0]);

        /* Read stderr then close. */
        bytes = read(errpipe[0], errbuff, CAPTMAX);
        errbuff[bytes] = 0;
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
