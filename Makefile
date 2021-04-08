CC = gcc
CFLAGS = -I. -Wall
IFLAGS = -as -br -brf -brs -ts4 -bli4 -i4 -di4 -npcs -nut
PREFIX = /usr/local
headers_exclude = common.h
headers = $(filter-out $(headers_exclude), $(wildcard *.h))
objects = $(headers:.h=.o)

httpload: main.c $(objects)
$(objects): %.o: %.c %.h

include tests/Makefile

.PHONY: clean
clean::
	-rm -f httpload *.o *.gcda *.gcno *.gcov *.c~

.PHONY: indent
indent::
	indent $(IFLAGS) *.c *.h 

indentbk = $(wildcard *.[hc]~)
.PHONY: indent-restore
indent-restore::
	for f in $(indentbk:%~=%); do \
		mv "$${f}~" "$${f}"; \
	done
