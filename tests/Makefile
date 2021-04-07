TEST_CFLAGS = -I./tests
testobjs := 
testobjs := $(testobjs:%=tests/%)
tests := $(wildcard tests/test_*.c)
tests := $(tests:%.c=%)

ifdef GCOV
CFLAGS += \
	-g \
	--coverage
endif


$(testobjs): %.o: %.c %.h
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -c -o $@ $<

#$(tests): %: %.c $(objects) $(testobjs)
$(tests): %: %.c $(objects)
	$(CC) $(TEST_CFLAGS) $(CFLAGS) -o $@ $^

.PHONY: test
test: $(tests)
	tests/test_cli

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
	-cd tests; rm test_cli *.o *.gcda *.gcno *.gcov *.c~

.PHONY: indent
indent::
	indent $(IFLAGS) tests/*.c tests/*.h 
