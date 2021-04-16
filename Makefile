CC = gcc
RM = rm -f
LDFLAGS = -lhttp_parser 
CFLAGS = \
		 -I. \
		 -Wall \
		 -fplan9-extensions \
		 -fms-extensions \
		 -D_GNU_SOURCE=

IFLAGS = \
		 -npcs \
		 -nut \
		 -as \
		 -bs \
		 -br \
		 -nce \
		 -ncdw \
		 -brf \
		 -brs \
		 -ts4 \
		 -bli4 \
		 -i4 \
		 -di4 \
		 -cli4 \
		 -sar \
		 -bc \
		 -bad \
		 -bap \
		 -bbb \
		 -sc \
		 -slc \
		 -sob

PREFIX = /usr/local
CLEAN = httploadc httploads *.o *.gcda *.gcno *.gcov *.c~ *.h~ coverage.info
client_headers := $(wildcard client*.h)
client_objects := $(client_headers:.h=.o)

server_headers := $(wildcard server*.h)
server_objects := $(server_headers:.h=.o)

headers_exclude := options.h common.h
common_headers := $(filter-out $(headers_exclude), $(wildcard *.h))
common_headers := $(filter-out $(server_headers), $(common_headers))
common_headers := $(filter-out $(client_headers), $(common_headers))
common_objects := $(common_headers:.h=.o)


all: httploadc httploads

$(common_objects) $(server_objects) $(client_objects): %.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ $< -c $(LDFLAGS)

httploadc: client_main.c $(client_objects) $(common_objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

httploads: server_main.c $(server_objects) $(common_objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

include tests/Makefile

.PHONY: clean
clean::
	-$(RM) -f $(CLEAN)

.PHONY: indent
indent::
	indent $(IFLAGS) *.c *.h 

indentbk = $(wildcard *.[hc]~)
.PHONY: indent-restore
indent-restore::
	for f in $(indentbk:%~=%); do mv "$${f}~" "$${f}"; done
