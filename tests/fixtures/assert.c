#include "fixtures/assert.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void
printbinary(const unsigned char *buf, int buflen) {
    int i;

    for (i = 0; i < buflen; i++) {
        printf("\\%02X", buf[i]);
    }
    printf("\n");
}

void
equalbin(const unsigned char *expected, const unsigned char *given,
         uint32_t len) {
    SUCCESS(memcmp(given, expected, len) == 0);

    /* Error */
    FAILED();
    EXPECTED();
    printbinary(expected, len);

    GIVEN();
    printbinary(given, len);

    exit(EXIT_FAILURE);
}

void
equalchr(const char expected, const char given) {
    SUCCESS(given == expected);

    /* Error */
    FAILED();
    EXPECTED();
    pdataln("%c", expected);

    GIVEN();
    pdataln("%c", given);

    exit(EXIT_FAILURE);
}

void
equalstr(const char *expected, const char *given) {
    SUCCESS(strcmp(given, expected) == 0);

    /* Error */
    FAILED();
    EXPECTED();
    pdataln("%s", expected);

    GIVEN();
    pdataln("%s", given);

    exit(EXIT_FAILURE);
}

void
equalnstr(const char *expected, const char *given, uint32_t len) {
    SUCCESS(strncmp(given, expected, len) == 0);

    /* Error */
    FAILED();
    EXPECTED();
    pdataln("%.*s", len, expected);

    GIVEN();
    pdataln("%.*s", len, given);

    exit(EXIT_FAILURE);
}

void
equalint(int expected, int given) {
    SUCCESS(given == expected);

    /* Error */
    FAILED();
    EXPECTED();
    pdataln("%d", expected);

    GIVEN();
    pdataln("%d", given);

    exit(EXIT_FAILURE);
}

void
notequalint(int expected, int given) {
    SUCCESS(given != expected);

    /* Error */
    FAILED();
    NOTEXPECTED();
    pdataln("%d", expected);

    GIVEN();
    pdataln("%d", given);

    exit(EXIT_FAILURE);
}

