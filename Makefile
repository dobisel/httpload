CC = gcc
CFLAGS = \
	-I. \
	-Wall 

PREFIX = /usr/local
SRC = .
BUILD = ./build
modules := logging httpload cli
objects := $(modules:%=$(BUILD)/%.o)


all: httpload


httpload: main.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^


$(BUILD)/%.o: $(SRC)/%.c $(SRC)/%.h
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<


.PHONY: clean
clean:
	-rm -r $(BUILD)
	-rm httpload


.PHONY: test
test:
	$(MAKE) -C tests test


.PHONY: coverage
coverage:
	$(MAKE) -C tests coverage
