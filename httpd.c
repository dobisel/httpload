#include "common.h"
#include "logging.h"
#include "ev.h"
#include "httpd.h"

/* Third party */
#include <http_parser.h>

#define HTTPRESP \
    "HTTP/1.1 %d %s" RN \
    "Server: httpload/" HTTPLOAD_VERSION RN \
    "Content-Length: %ld" RN \
    "Connection: %s" RN

static int
should_keepalive(const http_parser * parser) {
    if (parser->http_major > 0 && parser->http_minor > 0) {
        /* HTTP/1.1 */
        if (parser->flags & F_CONNECTION_CLOSE) {
            return 0;
        }
    }
    else {
        /* HTTP/1.0 or earlier */
        if (!(parser->flags & F_CONNECTION_KEEP_ALIVE)) {
            return 0;
        }
    }
    return 0;
}

static int
req_complete_cb(http_parser * p) {
    struct peer *c = (struct peer *) p->data;

    /* Req complete */
    c->state = PS_WRITE;
    return OK;
}

static int
body_cb(http_parser * p, const char *at, size_t len) {
    struct peer *c = (struct peer *) p->data;

    /* Request body */
    if (rb_write(&c->writerb, at, len)) {
        WARN("Buffer full");
        p->state = PS_CLOSE;
        return ERR;
    }
    return OK;
}

static int
headercomplete_cb(http_parser * p) {
    char tmp[2048];
    size_t tmplen;
    long clen = p->content_length;
    struct peer *c = (struct peer *) p->data;
    int keep = should_keepalive(p);

    tmplen = sprintf(tmp, HTTPRESP,
                     200, "OK",
                     clen > 0 ? clen : 15, keep ? "keep-alive" : "close");
    if (rb_write(&c->writerb, tmp, tmplen)) {
        WARN("Buffer full");
        p->state = PS_CLOSE;
        return ERR;
    }

    /* Write more headers */
    // ....

    /* Terminate headers by `\r\n` */
    if (rb_write(&c->writerb, RN, 2)) {
        p->state = PS_CLOSE;
        return ERR;
    }

    /* Parse header done, clen: %ld */
    if (clen <= 0) {
        tmplen = sprintf(tmp, "Hello HTTPLOAD!");
        if (rb_write(&c->writerb, tmp, tmplen)) {
            /* rbwrite error */
            p->state = PS_CLOSE;
            return ERR;
        }
        return OK;
    }

    p->state = PS_READ;
    return OK;
}

static http_parser_settings hpconf = {
    .on_headers_complete = headercomplete_cb,
    .on_body = body_cb,
    .on_message_complete = req_complete_cb,
};

static void
client_connected(struct evs *evs, struct peer *c) {
    /* Initialize the http parser for this connection. */
    struct http_parser *p = malloc(sizeof (struct http_parser));

    http_parser_init(p, HTTP_REQUEST);
    c->handler = p;
    c->state = PS_READ;
    p->data = c;
}

static void
data_recvd(struct ev *ev, struct peer *c, const char *data, size_t len) {
    struct http_parser *p = (struct http_parser *) c->handler;

    /* Parse */
    if (http_parser_execute(p, &hpconf, data, len) != len) {
        c->state = PS_CLOSE;
        return;
    }

    if (p->http_errno) {
        WARN("http-parser: %d: %s", p->http_errno,
             http_errno_name(p->http_errno));
        c->state = PS_CLOSE;
        return;
    }
}

static void
writefinish(struct ev *ev, struct peer *c) {
    struct http_parser *p = (struct http_parser *) c->handler;

    if (!http_should_keep_alive(p)) {
        c->state = PS_CLOSE;
        return;
    }
    c->state = PS_READ;
}

void
httpd_start(struct httpd *server) {
    server->on_recvd = data_recvd;
    server->on_writefinish = writefinish;
    server->on_connect = client_connected;
    ev_server_start((struct evs *) server);
}

int
httpd_stop(struct httpd *server) {
    return ev_server_terminate((struct evs *) server);
}

/** Cannot cover due the GCC will not gather info of fork() parents. */
// LCOV_EXCL_START
int
httpd_join(struct httpd *server) {
    return ev_server_join((struct evs *) server);
}
// LCOV_EXCL_END
