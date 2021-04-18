TEST_CFLAGS = -I./tests
TEST_LDFLAGS = -lcurl -lmimick
TEST_BIN = tests/bin
UNIT = % 

fixture_headers := $(wildcard tests/fixtures/*.h)
fixture_objects := $(fixture_headers:%.h=%.o)

client_tests := $(wildcard tests/test_client_*.c)
client_tests := $(client_tests:tests/%.c=$(TEST_BIN)/%)

server_tests := $(wildcard tests/test_server_*.c)
server_tests := $(server_tests:tests/%.c=$(TEST_BIN)/%)

common_tests := $(wildcard tests/test_*.c)
common_tests := $(common_tests:tests/%.c=$(TEST_BIN)/%)

common_tests := $(filter-out $(client_tests), $(common_tests))
common_tests := $(filter-out $(server_tests), $(common_tests))

nocover := \
	client_main.c \
	server_main.c

tocover := $(wildcard *.c)
tocover := $(filter-out $(nocover), $(tocover))
nocover := $(subst $(eval) ,;,$(nocover))

ifdef GCOV
CFLAGS += \
	--coverage \
	-fprofile-exclude-files="$(nocover)" \
	-g

endif

$(fixture_objects): %.o: %.c %.h
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -o $@ $< -c $(LDFLAGS) $(TEST_LDFLAGS)

$(client_tests): $(TEST_BIN)/%: tests/%.c \
		$(common_objects) $(client_objects) $(fixture_objects)
	mkdir -p $(TEST_BIN)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -o $@ $^ $(LDFLAGS) $(TEST_LDFLAGS)

$(server_tests): $(TEST_BIN)/%: tests/%.c \
		$(common_objects) $(server_objects) $(fixture_objects)
	mkdir -p $(TEST_BIN)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -o $@ $^ $(LDFLAGS) $(TEST_LDFLAGS)

$(common_tests): $(TEST_BIN)/%: tests/%.c \
		$(common_objects) $(fixture_objects)
	mkdir -p $(TEST_BIN)
	$(CC) $(CFLAGS) $(TEST_CFLAGS) -o $@ $^ $(LDFLAGS) $(TEST_LDFLAGS)

.PHONY: test
test: $(common_tests) $(client_tests) $(server_tests)
	@set -e; for t in $(filter $(TEST_BIN)/test_$(UNIT), $(common_tests)); \
		do "$${t}"; done
	@set -e; for t in $(filter $(TEST_BIN)/test_$(UNIT), $(client_tests)); \
		do "$${t}"; done
	@set -e; for t in $(filter $(TEST_BIN)/test_$(UNIT), $(server_tests)); \
		do "$${t}"; done

.PHONY: coverage
coverage: test
	lcov --capture \
		--directory . \
		--no-external \
		--output-file coverage.info
ifdef COVERALLS 
	coveralls -n -e tests 
endif

.PHONY: cover
cover:
	$(MAKE) coverage GCOV=1

coverage_html_index := $(shell readlink -f covout/index.html)

.PHONY: cover-html
cover-html: cover
	genhtml coverage.info --output-directory covout
	@echo "Open file://$(coverage_html_index) to view the coverage result."

.PHONY: cover-html-show
cover-html-show: cover-html
	xdg-open covout/index.html

.PHONY: coveralls
coveralls: 
	$(MAKE) coverage GCOV=1 COVERALLS=1

.PHONY: clean
clean::
	-cd tests; $(RM) $(CLEAN) bin/*
	-cd tests/fixtures; $(RM) $(CLEAN) 

.PHONY: indent
indent::
	indent $(IFLAGS) tests/*.c 

tindentbk = $(wildcard ./tests/*.[hc]~)

.PHONY: indent-restore
indent-restore::
	for f in $(tindentbk:%~=%); do \
		mv "$${f}~" "$${f}"; \
	done
