
ifdef GCOV

CFLAGS += \
	-I./include \
	-g \
	--coverage

endif

tests/test_cli: tests/test_cli.c $(objects)
	$(CC) -Itests/include $(CFLAGS) -o $@ $^

.PHONY: test
test: tests/test_cli
	tests/test_cli
	-cat stdout.tmp
	-rm stdout.tmp


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
	-cd tests; rm test_cli *.o *.gcda *.gcno *.gcov

