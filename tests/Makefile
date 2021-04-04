CC = gcc
CFLAGS = \
	-I$(SRC) \
	-I./include \
	-Wall \
	-g \
	--coverage \
	-fprofile-exclude-files="test_*" 


SRC = ..
BUILD = build
modules := logging httpload cli
objects := $(modules:%=$(BUILD)/%.o)


$(BUILD)/%.o: $(SRC)/%.c $(SRC)/%.h
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<


$(BUILD)/test_cli: test_cli.c $(objects)
	$(CC) $(CFLAGS) -o $@ $^


.PHONY: coverage
coverage: test
	gcov -o$(BUILD) $(SRC)/*.c


.PHONY: test
test: $(BUILD)/test_cli
	$(BUILD)/test_cli


.PHONY: clean
clean:
	-rm -r $(BUILD)
	-rm *.gcda *.gcno *.gcov

