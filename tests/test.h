#ifndef R_TEST_H
#define R_TEST_H
#include <stdio.h>
#include <time.h>

extern int total_asserts;
extern int total_passed;

#define Benchmark(name, times) for (int t = 1; t;) for (double final; t;) for(clock_t start = clock(); t; final = (double)(clock()-start) / CLOCKS_PER_SEC, printf("  <%s> %gs / %dtimes = %gs\n", name, final, times, final/times), t = 0) for (int i = 0; i < times; ++i)
#define Expect(condition) printf("  %s \x1b[37m%s\x1b[0m\n", (total_asserts += 1, (condition)) ? (total_passed += 1, "\x1b[32m[PASS]\x1b[0m") : "\x1b[31m[FAIL]\x1b[0m", #condition);
#define Info(...) do { printf("  \x1b[35m[INFO] \x1b[37m"); printf(__VA_ARGS__); printf("\x1b[0m\n"); } while(0)
static inline int Percent_(int a, int b)
{
    if (b == 0)
        return 100;
    return ((float)a / b) * 100;
}

static inline void Tally() 
{
    printf("%i out of %i assertions passed (%d%%)\n", total_passed, total_asserts, Percent_(total_passed, total_asserts));
}

static inline void TallyBench() 
{
    printf("Benchmark stub\n");
}
#endif