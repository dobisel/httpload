#ifndef FIXTURES_ASSERT_H
#define FIXTURES_ASSERT_H

#include "logging.h"

#include <sys/types.h>
#include <stdbool.h>

/* third-party */
#include <mimick.h>

struct test {
    const char *filename;
    const char *func;
    size_t line;
    char *tmp;
    size_t tmplen;
};

void isnull(struct test *t, bool not, void *given);
void eqbin(struct test *t, bool not, size_t len, const char *given, 
        const char *expected);
void eqstr(struct test *t, bool not, const char *given, const char *expfmt, 
        ...);
void eqnstr(struct test *t, bool not, size_t len, const char *given, 
        const char *expfmt, ...);
void eqint(struct test *t, bool not, int given, int expected);
void pre_assert(struct test *t, const char *func, size_t line);
struct test * test_suite_setup(loglevel_t loglevel, const char *filename);
int test_suite_teardown(struct test *t);

/* Private macros. */
#define __PA() pre_assert(t, __func__, __LINE__)

/* Public macros. */
#define TEST_CASE static
#define TEST_BEGIN(l) test_suite_setup(l, __FILE__)

#define TEST_CLEAN(t) test_suite_teardown(t)

/** Is NULL. */
#define    ISNULL(g) __PA(); isnull(t, false, g)
#define ISNOTNULL(g) __PA(); isnull(t, true,  g)

/** Equal int */
#define  EQI(g, e) __PA(); eqint((t), false, (g), (e))
#define NEQI(g, e) __PA(); eqint((t), true,  (g), (e))

/** Equal str */
#define  EQS(g, e, ...)     __PA(); eqstr (t, false,    g, e, ## __VA_ARGS__)
#define NEQS(g, e, ...)     __PA(); eqstr (t, true,     g, e, ## __VA_ARGS__)
#define  EQNS(n, g, e, ...) __PA(); eqnstr(t, false, n, g, e, ## __VA_ARGS__)
#define NEQNS(n, g, e, ...) __PA(); eqnstr(t, true,  n, g, e, ## __VA_ARGS__)

/** Equal binary */
#define  EQB(n, g, e, ...)  __PA(); eqbin (t, false, n, g, e, ## __VA_ARGS__)
#define NEQB(n, g, e, ...)  __PA(); eqbin (t, true,  n, g, e, ## __VA_ARGS__)

/** Mimick */
#define MMKOK(m, t)         __PA(); EQI(mmk_verify(m, .times = t), 1)
#define MMK_WHEN_RET(m, r, e) mmk_when(m,.then_return = r,.then_errno = e)
#define MMK_WHEN_CALL(m, c, e) \
    mmk_when(m, .then_errno = e, .then_call = (mmk_fn)c)
#define MMK_RESET(f) mmk_reset(f)
#define MMK_DEFINE(...) mmk_mock_define(__VA_ARGS__)
#endif
