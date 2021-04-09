CC = gcc
CFLAGS = -I. -Wall -fplan9-extensions
IFLAGS = -as -br -brf -brs -ts4 -bli4 -i4 -di4 -npcs -nut
PREFIX = /usr/local

client_headers := $(wildcard client*.h)
client_objects := $(client_headers:.h=.o)

server_headers := $(wildcard server*.h)
server_objects := $(server_headers:.h=.o)

headers_exclude := common.h
common_headers := $(filter-out $(headers_exclude), $(wildcard *.h))
common_headers := $(filter-out $(server_headers), $(common_headers))
common_headers := $(filter-out $(client_headers), $(common_headers))
common_objects := $(common_headers:.h=.o)


all: httploadc httploads

httploadc: client_main.c $(client_objects) $(common_objects)
	$(CC) $(CFLAGS) -o $@ $^

httploads: server_main.c $(server_objects) $(common_objects)
	$(CC) $(CFLAGS) -o $@ $^

$(common_objects): %.o: %.c %.h

include tests/Makefile

.PHONY: clean
clean::
	-rm -f httploadc httploads *.o *.gcda *.gcno *.gcov *.c~ *.h~

.PHONY: indent
indent::
	indent $(IFLAGS) *.c *.h 

indentbk = $(wildcard *.[hc]~)
.PHONY: indent-restore
indent-restore::
	for f in $(indentbk:%~=%); do mv "$${f}~" "$${f}"; done
