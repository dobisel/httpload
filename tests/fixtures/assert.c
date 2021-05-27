#include "logging.h"
#include "options.h"
#include "ansicolors.h"
#include "fixtures/assert.h"

#include <stdio.h>
#include <stdarg.h>

#define TEST_TEMP_BUFFSIZE     (EV_WRITE_BUFFSIZE * 2)
#define FUNCSIGN()      printf(BHGRN "." RST)
#define PASS()          printf(GRN "." RST)
#define FAIL_HEADER() printf(\
    N HYEL "%s" HBLU ":" HGRN "%lu" HBLU ": " \
    HMAG "%s" WHT "(" BLU "struct" RST " test *t) " HRED "Failed!" \
            RST N, t->filename, t->line, t->func)
#define FAIL(fmt, not, g, e, compare) ({ FAIL_HEADER(); \
    if (compare) { \
        if (not) printf("NOT "); \
        printf(GRN "EXPECTED:\t" RST fmt N, e);} \
    printf(YEL "GIVEN:\t\t" RST fmt N, g); \
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
        ERRORX("invalid format string: %s", expfmt);
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
    FAIL("%s", not, given, t->tmp, true);
}

void 
eqstr(struct test *t, bool not, const char *given, 
        const char *expfmt, ...) {
    va_list args;
    
    va_start(args, expfmt);
    t->tmplen = vsprintf(t->tmp, expfmt, args);
    if (t->tmplen < 0) {
        ERRORX("invalid format string: %s", expfmt);
    }
    va_end(args);
    
    if (not ^ (strcmp(given, t->tmp) == 0)) {
        PASS();
        return;
    }
    FAIL("%s", not, given, t->tmp, true);
}

void 
eqint(struct test *t, bool not, int given, int expected) {
    if (not ^ (given == expected)) {
        PASS();
        return;
    }
    FAIL("%d", not, given, expected, true);
}

void 
isnull(struct test *t, bool not, void *given) {
    if (not ^ (given == NULL)) {
        PASS();
        return;
    }
    FAIL("%p", not, given, NULL, false);
}

void 
pre_assert(struct test *t, const char *func, size_t line) {
    if (t->func == NULL || strcmp(func, t->func) != 0) {
        FUNCSIGN();
    }
    t->func = func;
    t->line = line;
}

#define TEST_MAXFILENAME    32
#define TM_PLUS             23
#define TM     (TEST_MAXFILENAME + TM_PLUS)

void
test_setup(struct test *t, const char *filename) {
    char *fn = basename(filename) + 5;
    t->func = NULL;
    t->line = 0;
    t->filename = filename;
    t->tmp = malloc(TEST_TEMP_BUFFSIZE);
    t->tmplen = 0;
    char temp[TM + 1];
    temp[TM] = 0;
    memset(temp, ' ', TM);
    sprintf(temp + (TM - (strlen(fn) + TM_PLUS)), 
            CYN"%.*s" BLU ": " RST, (int)strlen(fn) - 2, fn);
    printf("%s", temp);
}

int
test_teardown(struct test *t) {
    free(t->tmp);
    if (t->func != NULL) {
        FUNCSIGN();
    }
    printf(N);
    return EXIT_SUCCESS;
}
