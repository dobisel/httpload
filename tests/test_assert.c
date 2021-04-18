#include "fixtures/assert.h"

#include <stdlib.h>

void
test_eqint(struct test *t) {
    EQI(1, 1);
    NEQI(1, 2);
}

void
test_eqstr(struct test *t) {
    EQS("foo 02", "%s %02d", "foo", 2);
    NEQS("foo 02", "%s %02d", "foo", 3);
}

void
test_eqnstr(struct test *t) {
    EQNS(5, "foo 02", "foo 03");
    EQNS(5, "foo 02", "foo 034567");
    EQNS(5, "foo 023456", "foo 03");

    NEQNS(4, "foo 02", "foo");
    NEQNS(4, "foo 02", "foo-");
    NEQNS(4, "foo", "foo 02");
}

void
test_eqbin(struct test *t) {
    EQB(3, "foo", "foo");
    NEQB(3, "foo", "bar");
}

void
test_isnull(struct test *t) {
    ISNULL(NULL);
    ISNOTNULL(t);
}

int
main() {
    struct test t;

    SETUP(&t);
    test_eqint(&t);
    test_eqstr(&t);
    test_eqnstr(&t);
    test_eqbin(&t);
    test_isnull(&t);
    return TEARDOWN(&t);
}
