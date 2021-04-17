#include "logging.h"
#include "ansicolors.h"
#include "fixtures/assert.h"

#include <stdio.h>
#include <stdarg.h>

#define TEST_TEMP_BUFFSIZE      1024 * 8
#define PASS()  printf(GRN "." RST)
#define FAIL_HEADER() \
    printf(N HYEL "%s" HBLU ":" HGRN "%lu" HBLU ": " MAG "%s Failed" RST N, \
                t->filename, t->line, t->func)
#define FAIL(fmt, not, g, e) ({ FAIL_HEADER(); \
    if (not) printf("NOT "); \
    printf("EXPECTED:\t" fmt N, e); \
    printf("GIVEN:\t\t" fmt N, g); \
    exit(EXIT_FAILURE); \
    })

static void
printbinary(const char *buf, int buflen) {
    for (int i = 0; i < buflen; i++) {
        printf("\\%02X", buf[i]);
    }
    printf(N);
}

void 
eqbin(struct test *t, bool not, size_t len, const char *given, 
        const char *expected) {
    
    if (not ^ (memcmp(given, expected, len) == 0)) {
        PASS();
        return;
    }
    FAIL_HEADER();
    if (not) {
        printf("NOT ");
    }
    printf("EXPECTED:\t"); 
    printbinary(expected, len);
    printf("GIVEN:\t\t"); 
    printbinary(given, len);
    exit(EXIT_FAILURE);
}

void 
eqnstr(struct test *t, bool not, size_t len, const char *given, 
        const char *expfmt, ...) {
    va_list args;
    
    va_start(args, expfmt);
    t->tmplen = vsnprintf(t->tmp, len + 1, expfmt, args);
    if (t->tmplen < 0) {
        ERRX("invalid format string: %s", expfmt);
    }
    va_end(args);
    
    if (t->tmplen > len) {
        t->tmplen = len;
        t->tmp[len] = 0;
    }
    if (not ^ (strncmp(t->tmp, given, len) == 0)) {
        PASS();
        return;
    }
    FAIL("%s", not, given, t->tmp);
}


void 
eqstr(struct test *t, bool not, const char *given, 
        const char *expfmt, ...) {
    va_list args;
    
    va_start(args, expfmt);
    t->tmplen = vsprintf(t->tmp, expfmt, args);
    if (t->tmplen < 0) {
        ERRX("invalid format string: %s", expfmt);
    }
    va_end(args);
    
    if (not ^ (strcmp(given, t->tmp) == 0)) {
        PASS();
        return;
    }
    FAIL("%s", not, given, t->tmp);
}


void 
eqint(struct test *t, bool not, int given, int expected) {
    if (not ^ (given == expected)) {
        PASS();
        return;
    }
    FAIL("%d", not, given, expected);
}

void 
pre_assert(struct test *t, const char *func, size_t line) {
    if (t->func == NULL) {
        printf("[");
    }
    else if (strcmp(func, t->func) != 0) {
        printf("] [");
    }
    t->func = func;
    t->line = line;
}

void
test_setup(struct test *t, const char *filename) {
    t->func = NULL;
    t->line = 0;
    t->filename = filename;
    t->tmp = malloc(TEST_TEMP_BUFFSIZE);
    t->tmplen = 0;
    log_setlevel(LL_DEBUG);
    printf(CYN "%s" BLU ": " RST, t->filename);
}

int
test_teardown(struct test *t) {
    free(t->tmp);
    if (t->func != NULL) {
        printf("]");
    }
    printf(N);
    return EXIT_SUCCESS;
}
