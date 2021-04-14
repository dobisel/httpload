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

    return EV_MOD_WRITE(c);
}

static int
body_cb(http_parser * p, const char *at, size_t len) {
    struct peer *c = (struct peer *) p->data;
    int err = rb_write(&c->resprb, at, len);

    if (err) {
        ERROR("Buffer full");
        return err;
    }
    return EV_MOD_WRITE(c);
}

static int
headercomplete_cb(http_parser * p) {
    int err;
    char tmp[2048];
    size_t tmplen;
    long clen = p->content_length;
    struct peer *c = (struct peer *) p->data;
    int keep = should_keepalive(p);

    tmplen = sprintf(tmp, HTTPRESP,
                     200, "OK",
                     clen > 0 ? clen : 15, keep ? "keep-alive" : "close");
    err = rb_write(&c->resprb, tmp, tmplen);
    if (err) {
        return err;
    }

    /* Write more headers */
    // ....

    /* Terminate headers by `\r\n` */
    err = rb_write(&c->resprb, RN, 2);
    if (err) {
        return err;
    }

    if (clen <= 0) {
        tmplen = sprintf(tmp, "Hello HTTPLOAD!");
        return rb_write(&c->resprb, tmp, tmplen);
    }
    return OK;
}

static http_parser_settings hpconf = {
    .on_headers_complete = headercomplete_cb,
    .on_body = body_cb,
    .on_message_complete = req_complete_cb,
};

static int
client_connected(struct ev *ev, struct peer *c) {
    /* Initialize the http parser for this connection. */
    struct http_parser *p = malloc(sizeof (struct http_parser));

    c->handler = p;
    http_parser_init(p, HTTP_REQUEST);
    p->data = c;
    return OK;
}

static int
data_recvd(struct ev *ev, struct peer *c, const char *data, size_t len) {
    struct http_parser *p = (struct http_parser *) c->handler;

    if (http_parser_execute(p, &hpconf, data, len) != len) {
        return ERR;
    }

    if (p->http_errno) {
        ERROR("http-parser: %d: %s", p->http_errno,
              http_errno_name(p->http_errno));
        return ERR;
    }
    return OK;
}

static int
write_finish(struct ev *ev, struct peer *c) {
    struct http_parser *p = (struct http_parser *) c->handler;

    if (!http_should_keep_alive(p)) {
        return ERR;
    }
    return OK;
}

int
httpd_start(struct httpd *server) {
    server->on_recvd = data_recvd;
    server->on_writefinish = write_finish;
    server->on_connect = client_connected;
    return evs_fork((struct evs *) server);
}
