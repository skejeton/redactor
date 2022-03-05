#include <stdio.h>

extern int total_asserts;
extern int total_passed;
#define run_test(call) do { printf("%s:\n", #call); call; } while(0)
#define expect(condition) printf("  %s \x1b[37m%s\x1b[0m\n", (total_asserts += 1, condition) ? (total_passed += 1, "\x1b[32m[PASS]\x1b[0m") : "\x1b[31m[FAIL]\x1b[0m", #condition);
#define info(...) do { printf("  \x1b[35m[INFO] \x1b[37m"); printf(__VA_ARGS__); printf("\x1b[0m\n"); } while(0)
static inline int percent_(int a, int b)
{
    if (b == 0)
        return 100;
    return ((float)a / b) * 100;
}

static inline void tally() 
{
    printf("%i out of %i assertions passed (%d%%)\n", total_passed, total_asserts, percent_(total_passed, total_asserts));
}