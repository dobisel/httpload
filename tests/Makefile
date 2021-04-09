TEST_CFLAGS = -I./tests

fixture_headers := $(wildcard tests/fixture*.h)
fixture_objects := $(fixture_headers:%.h=%.o)

client_tests := $(wildcard tests/test_client_*.c)
client_tests := $(client_tests:%.c=%)

server_tests := $(wildcard tests/test_server_*.c)
server_tests := $(server_tests:%.c=%)

ifdef GCOV
CFLAGS += \
	-g \
	--coverage
endif

show:
	@echo $(fixture_objects)

tests/httpmock: tests/httpmock.c
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -o $@ $^

$(fixture_objects): %.o: %.c %.h
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -c -o $@ $<

$(client_tests): %: %.c $(common_objects) $(client_objects) $(fixture_objects)
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -o $@ $^

$(server_tests): %: %.c $(common_objects) $(server_objects) $(fixture_objects)
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -o $@ $^

.PHONY: client_test
client_test: $(client_tests)
	for t in $(client_tests); do "$${t}"; done

.PHONY: server_test
server_test: $(server_tests)
	for t in $(server_tests); do "$${t}"; done

.PHONY: test
test: server_test client_test

.PHONY: coverage
coverage: test
ifdef COVERALLS 
	coveralls -e tests
else
	gcov *.c
endif

.PHONY: cover
cover:
	$(MAKE) coverage GCOV=1

.PHONY: coveralls
coveralls: 
	$(MAKE) coverage GCOV=1 COVERALLS=1

.PHONY: clean
clean::
	-cd tests; rm $(server_tests) $(client_tests) \
		*.o *.gcda *.gcno *.gcov *.c~ *.h~

.PHONY: indent
indent::
	indent $(IFLAGS) tests/*.c tests/*.h 

tindentbk = $(wildcard ./tests/*.[hc]~)

.PHONY: indent-restore
indent-restore::
	for f in $(tindentbk:%~=%); do \
		mv "$${f}~" "$${f}"; \
	done
