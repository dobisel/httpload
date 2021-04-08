#include "cli.h"
#include "testing.h"
#include "logging.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <argp.h>


#define STDOUT  1
#define STDERR  2
#define PROG    "httpload"
#define CAPTMAX 1024


static int
execchild(int argc, char **argv) {
    char **newargv = malloc(sizeof(char *) * (argc + 1));
    newargv[0] = PROG;
    memcpy(newargv + 1, argv, argc * sizeof(char *));
    argc++;

    int status = cli_run(argc, newargv);
    free(newargv);
    return status;
}


int
capture(int argc, char **argv, char *const out, char *const err) {
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
        status = execchild(argc, argv);
        close(outpipe[1]);
        close(errpipe[1]);
        exit(status);
    }
    return WEXITSTATUS(status);
}


void
test_version() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = capture(1, (char *[]) { "--version" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION CR, out);

    status = capture(1, (char *[]) { "-V" }, out, err);
    eqint(0, status);
    eqstr(HTTPLOAD_VERSION CR, out);
}


void
test_verbosity() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    status = capture(2, (char *[]) { "-v", "0" }, out, err);
    eqint(0, status);
    eqstr("", out);
    eqstr("", err);
}

void
test_invalidargument() {
    char out[CAPTMAX + 1] = { 0 };
    char err[CAPTMAX + 1] = { 0 };
    int status;

    /* Invalid optional argument. */
    status = capture(2, (char *[]) { "--invalidargument", "0" }, out, err);
    eqint(EINVAL, status);
    eqstr("", out);
    eqnstr("httpload: unrecognized option '--invalidargument'", err, 49);

    /* Invalid positional arguments. */
    status = capture(3, (char *[]) { "foo", "bar", "baz" }, out, err);
    eqint(EINVAL, status);
    eqstr("", out);
    eqnstr("httpload: Too many arguments", err, 28);
}


int
main() {
    log_setlevel(LOG_DEBUG);
    test_version();
    test_verbosity();
    test_invalidargument();
    return EXIT_SUCCESS;
}
