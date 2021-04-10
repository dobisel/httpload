TEST_CFLAGS = -I./tests

fixture_headers := $(wildcard tests/fixtures/*.h)
fixture_objects := $(fixture_headers:%.h=%.o)

client_tests := $(wildcard tests/test_client_*.c)
client_tests := $(client_tests:%.c=%)

server_tests := $(wildcard tests/test_server_*.c)
server_tests := $(server_tests:%.c=%)

common_tests := $(wildcard tests/test_*.c)
common_tests := $(common_tests:%.c=%)
common_tests := $(filter-out $(client_tests), $(common_tests))
common_tests := $(filter-out $(server_tests), $(common_tests))


ifdef GCOV
CFLAGS += \
	-g \
	--coverage
endif

show:
	@echo $(fixture_objects)
	@echo $(common_tests)

tests/httpmock: tests/httpmock.c
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -o $@ $^

$(fixture_objects): %.o: %.c %.h
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -c -o $@ $<

$(client_tests): %: %.c $(common_objects) $(client_objects) $(fixture_objects)
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -o $@ $^

$(server_tests): %: %.c $(common_objects) $(server_objects) $(fixture_objects)
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -o $@ $^

$(common_tests): %: %.c $(common_objects) $(fixture_objects)
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -o $@ $^


.PHONY: test
test: $(common_tests) $(client_tests) $(server_tests) 
	for t in $(common_tests); do "$${t}"; done
	for t in $(client_tests); do "$${t}"; done
	for t in $(server_tests); do "$${t}"; done

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
	-cd tests; rm $(server_tests) $(client_tests) *.o *.gcda *.gcno *.gcov \
		*.c~ *.h~ 
	-cd tests/fixtures; rm *.o *.gcda *.gcno *.gcov *.c~ *.h~

.PHONY: indent
indent::
	indent $(IFLAGS) tests/*.c tests/*.h 

tindentbk = $(wildcard ./tests/*.[hc]~)

.PHONY: indent-restore
indent-restore::
	for f in $(tindentbk:%~=%); do \
		mv "$${f}~" "$${f}"; \
	done
