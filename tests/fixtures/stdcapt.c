#include "logging.h"
#include "fixtures/stdcapt.h"

#include <stdlib.h>
#include <unistd.h>

static void 
_capt_start(struct captfile *cf) {
    int newfd;
    char template[64];

    sprintf(template, "capt-%s-XXXXXX.txt", cf->name);
    
    cf->backfd = dup(cf->fd);
    
    /* Creating a tempfile */
    newfd = mkstemps(template, 4);
    
    /* Duplicate tempfile */
    dup2(newfd, cf->fd);
    
    /* Close tempfile */
    close(newfd);
}

static void 
_capt_restore(struct captfile *cf, char *outbuff) {
    int len;
    int readfd = dup(cf->fd);
   
    dup2(cf->backfd, cf->fd);
    close(cf->backfd);
    
    lseek(readfd, 0, SEEK_SET);
    len = read(readfd, outbuff, STDCAPTMAX);
    if (len == ERR) {
        ERRORX("Cannot read from fd: %d", readfd);
    }
    
    outbuff[len] = 0;
    close(readfd);
}

void
capt_start(struct capt *c, enum captopt options) {
    c->options = options;
    c->out[0] = 0;
    c->err[0] = 0;
    c->running = true;

    /* stdout */
    if (options != STDCAPT_NO_OUT) {
        strcpy(c->stdout.name, "stdout");
        c->stdout.fd = STDOUT_FILENO;
        _capt_start(&c->stdout);
    }

    /* stderr */
    if (options != STDCAPT_NO_ERR) {
        strcpy(c->stderr.name, "stderr");
        c->stderr.fd = STDERR_FILENO;
        _capt_start(&c->stderr);
    }
}

void
capt_restore(struct capt *c) {
    if (!c->running) {
        return;
    }

    /* stdout */
    if (c->options != STDCAPT_NO_OUT) {
        _capt_restore(&c->stdout, c->out);
    }

    /* stderr */
    if (c->options != STDCAPT_NO_ERR) {
        _capt_restore(&c->stderr, c->err);
    }

    c->running = false;
}
