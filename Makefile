CC = gcc
CFLAGS = -I.
PREFIX = /usr/local
BUILD := ./build

COMMON_DEPS := logging.h common.h
MODS := logging httpload
OBJS := $(MODS:%=$(BUILD)/%.o)


all: httpload


httpload: cli.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^


$(BUILD)/%.o : %.c %.h $(COMMON_DEPS)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<


.PHONY: clean
clean:
	rm -r $(BUILD) httpload

