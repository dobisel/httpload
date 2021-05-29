#include "common.h"
#include "logging.h"
#include "ev.h"
#include "httpd.h"

#define HTTPD_RESP_HEADERS \
    "Server: httpload/" HTTPLOAD_VERSION RN \
    "Content-Length: %ld" RN \
    "Connection: %s" RN

static int
_should_keepalive(const http_parser * parser) {
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

#define resp_write(c, s, l) rb_write(&c->writerb, s, l)

static int
_resp_write_statusline(struct peer *c, const char *status) {
    return resp_write(c, "HTTP/1.1 ", 9)
        | resp_write(c, status, strlen(status))
        | resp_write(c, RN, 2);
}

static int
_resp_error(struct peer *c, const char *status) {
    struct http_parser *p = (struct http_parser *) c->handler;

    RB_RESET(&c->writerb);
    p->flags |= F_CONNECTION_CLOSE;
    return _resp_write_statusline(c, status);
}

static int
_body_cb(http_parser * p, const char *at, size_t len) {
    struct peer *c = (struct peer *) p->data;

    /* Request body: %ld */
    return resp_write(c, at, len);
}

static int
_headercomplete_cb(http_parser * p) {
    struct peer *c = (struct peer *) p->data;
    char tmp[2048];
    size_t tmplen;
    ssize_t clen = p->content_length;
    int keep = _should_keepalive(p);

    /* Status line */
    _resp_write_statusline(c, "200 Ok");

    tmplen = sprintf(tmp, HTTPD_RESP_HEADERS, clen > 0 ? clen : 15,
                     keep ? "keep-alive" : "close");
    resp_write(c, tmp, tmplen);

    /* Write more headers */
    // ....

    /* Terminate headers by `\r\n` */
    resp_write(c, RN, 2);

    /* Parse header done, clen: %ld */
    if (clen <= 0) {
        resp_write(c, "Hello HTTPLOAD!", 15);
        return OK;
    }

    if (RB_AVAILABLE(&c->writerb) < clen) {
        _resp_error(c, "413 Request Entity Too Large");
    }
    return OK;
}

static int
_req_complete_cb(http_parser * p) {
    struct peer *c = (struct peer *) p->data;

    /* Req complete */
    c->status = PS_WRITE;
    return OK;
}

static int
_header_value_cb(http_parser * p, const char *at, size_t len) {
    struct peer *c = (struct peer *) p->data;

    /* Check for 100 continue */
    if (strncmp(at, "100-continue", len) == 0) {
        /* HTTP 100 continue */
        resp_write(c, "HTTP/1.1 100 Continue" RN RN, 25);
        c->status = PS_WRITE;
    }
    return OK;
}

static http_parser_settings _hpconf = {
    .on_header_value = _header_value_cb,
    .on_headers_complete = _headercomplete_cb,
    .on_body = _body_cb,
    .on_message_complete = _req_complete_cb,
};

static void
_client_connected(struct evs *evs, struct peer *c) {
    /* Initialize the http parser for this connection. */
    struct http_parser *p = malloc(sizeof (struct http_parser));

    RB_RESET(&c->writerb);
    http_parser_init(p, HTTP_REQUEST);
    c->handler = p;
    c->status = PS_READ;
    p->data = c;
}

static void
_client_disconnected(struct evs *evs, struct peer *c) {
    /* Deinitialize the http parser for this connection. */
    struct http_parser *p = (struct http_parser *) c->handler;

    free(p);
}

static void
_data_recvd(struct ev *ev, struct peer *c, const char *data, size_t len) {
    struct http_parser *p = (struct http_parser *) c->handler;

    /* Parse */
    http_parser_execute(p, &_hpconf, data, len);
    if (p->http_errno) {
        WARNN("http-parser: %d: %s", p->http_errno,
              http_errno_name(p->http_errno));
        _resp_error(c, "400 Bad Request");
        c->status = PS_WRITE;
    }
}

static void
_writefinish(struct ev *ev, struct peer *c) {
    struct http_parser *p = (struct http_parser *) c->handler;

    if (!http_should_keep_alive(p)) {
        /* Closing Connection */
        c->status = PS_CLOSE;
        return;
    }

    /* More Reading */
    c->status = PS_READ;
}

int
httpd_start(struct httpd *server) {
    http_parser_set_max_header_size(server->max_headers_size);
    server->on_recvd = _data_recvd;
    server->on_writefinish = _writefinish;
    server->on_connect = _client_connected;
    server->on_disconnect = _client_disconnected;
    return ev_server_start((struct evs *) server);
}

int
httpd_stop(struct httpd *server) {
    return ev_server_terminate((struct evs *) server);
}

int
httpd_join(struct httpd *server) {
    return ev_server_join((struct evs *) server);
}
