//#ifndef FIXTURES_ASSERT_H
//#define FIXTURES_ASSERT_H
//
//#include <stdint.h>
//
//#define RED     "\e[0;31m"
//#define GREEN   "\e[0;32m"
//#define YELLOW  "\e[0;33m"
//#define BLUE    "\e[0;34m"
//#define MAGENTA "\e[0;35m"
//#define CYAN    "\e[0;36m"
//#define WHITE   "\e[0;37m"
//
//#define CLR     "\33[0m"
//
//#define pcolor(c_, f_, ...) \
//    printf(c_); \
//    printf((f_), ##__VA_ARGS__); \
//    printf(CLR)
//
//#define pok(f_, ...) pcolor(GREEN, f_, ##__VA_ARGS__)
//#define perr(f_, ...) pcolor(RED, f_, ##__VA_ARGS__)
//
//#define pokln(f_, ...) pok(f_, ##__VA_ARGS__); printf("\r\n")
//#define perrln(f_, ...) perr(f_, ##__VA_ARGS__); printf("\r\n")
//#define pcolorln(c_, f_, ...) pcolor(c_, f_, ##__VA_ARGS__); printf("\r\n")
//
//#define pdataln(f_, ...) pcolorln(WHITE, f_, ##__VA_ARGS__);
//
//#define SUCCESS(c) if (c) {pokln("%s Ok", __func__); return; }
//#define FAILED() perrln("%s Failed", __func__)
//#define EXPECTED() pcolor(BLUE, "Expected: ")
//#define GIVEN() pcolor(YELLOW, "Given: ")
//#define NOTEXPECTED() pcolor(CYAN, "Not Expected: ")
//
//#define assert(f, ...) \
//    pcolor(CYAN, "%s:%d", __FILE__, __LINE__); \
//    pcolor(MAGENTA, " [%s] ", __func__); \
//    f(__VA_ARGS__)
//
//#define eqchr(...) assert(equalchr, __VA_ARGS__)
//#define eqstr(...) assert(equalstr, __VA_ARGS__)
//#define eqnstr(...) assert(equalnstr, __VA_ARGS__)
//#define eqint(...) assert(equalint, __VA_ARGS__)
//#define neqint(...) assert(notequalint, __VA_ARGS__)
//#define eqbin(e, g, l) \
//    assert(equalbin, (unsigned char*)g, (unsigned char*)e, l)
//
//void equalbin(const unsigned char *expected, const unsigned char *given,
//         uint32_t len);
//void equalchr(const char expected, const char given);
//void equalstr(const char *expected, const char *given);
//void equalnstr(const char *expected, const char *given, uint32_t len);
//void equalint(int expected, int given);
//void notequalint(int expected, int given);
// 
//#endif
