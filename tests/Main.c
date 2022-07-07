#include "test.h"
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

void Test_Buffer_Main();
void Test_BufferTape_Main();
void Test_Redex_Main();
void Test_Redex_Compiler_Main();
void Bench_Redex_Main();

int total_asserts, total_passed;

struct {
    int argc;
    char **argv;
}
typedef Arguments;

struct {
    const char *key;
    const char *val;
}
typedef KeyValue;

static char *GetArg(Arguments *args, int n)
{
    if (n < 0 || n >= args->argc) {
        return NULL;
    }
    return args->argv[n];
}

static char *NextArg(Arguments *args)
{
    char *arg_value = GetArg(args, 0);
    if (arg_value) {
        args->argc -= 1;
        args->argv += 1;
    }
    return arg_value;
}

static KeyValue ParseArg(Arguments *args)
{
    // skip until we find hyphen
    while (args->argc) {
        if (GetArg(args, 0)[0] == '-') {
            const char *value = GetArg(args, 1);
            // check if another hyphen argument
            if (value && value[0] != '-') {
                return (KeyValue){NextArg(args)+1 /* sans the hyphen */, NextArg(args)};
            } else {
                return (KeyValue){NextArg(args)+1 /* sans the hyphen */, NULL};
            }
        }
        NextArg(args);
    }
    return (KeyValue){NULL, NULL};
}

#define IterArgs(arg, argc, argv) for (Arguments args_ = {argc, argv}; args_.argc > 0;) for(KeyValue arg; args_.argc > 0; args_.argc = 0) while (arg = ParseArg(&args_), arg.key)

struct {
    char *name;
    void (*test)();
    void (*bench)();
}
typedef Test;

Test const static tests[] = {
    {"buffer", Test_Buffer_Main},
    {"buffertape", Test_BufferTape_Main},
    {"redex", Test_Redex_Main, Bench_Redex_Main},
    {"redex_compiler", Test_Redex_Compiler_Main}
};

const int tests_len = sizeof tests / sizeof tests[0];

void RunTest(Test const *t)
{
    if (t->test == NULL) {
        fprintf(stderr, "Error: `%s` doesn't have a test\n", t->name);
        exit(-1);
    }
    printf("%s:\n", t->name);
    t->test();
}

void RunBench(Test const *t)
{
    if (t->bench == NULL) {
        fprintf(stderr, "Error: `%s` doesn't have a benchmark\n", t->name);
        exit(-1);
    }
    printf("%s:\n", t->name);
    t->bench();
}

char *CopyLowerString(const char *s)
{
    const int string_length = strlen(s);
    char *new_string = malloc(string_length + 1);

    for (int i = 0; i < string_length; ++i) {
        new_string[i] = tolower(s[i]);
    }

    // write nul
    new_string[string_length] = 0;
    return new_string;
}

// NOTE(skejeton): this function isn't efficent, but it does it's job for handling arguments ok
const bool StrEqualNocase(const char *a, const char *b)
{
    char *left = CopyLowerString(a), *right = CopyLowerString(b);
    bool equal = strcmp(left, right) == 0;
    free(left);
    free(right);
    return equal;
}

const Test *FindTest(const char *name)
{
    const Test *output = NULL;

    for (int i = 0; output == NULL && i < tests_len; ++i) {
        if (StrEqualNocase(tests[i].name, name)) {
            output = &tests[i];
        }
    }

    // TODO(skejeton): maybe fuzzy search on fail?
    return output;
}

void HandleMode_Test(const char *test_name)
{
    if (test_name != NULL) {
        const Test *test = FindTest(test_name);
        if (test != NULL) {
            RunTest(test);
        } else {
            fprintf(stderr, "Error: Can't find test %s! Use `-mode list` argument to list tests\n", test_name);
            return;
        }
    } else {
        for (int i = 0; i < tests_len; ++i) {
            if (tests[i].test != NULL) {
                RunTest(&tests[i]);
            }
        }
    }

    Tally();
}

void HandleMode_Bench(const char *bench_name)
{
    #ifdef CFLAGS
    printf("Benchmarking with flags `%s`\n", CFLAGS);
    #endif
    if (bench_name != NULL) {
        const Test *test = FindTest(bench_name);
        if (test != NULL) {
            RunBench(test);
        } else {
            fprintf(stderr, "Error: Can't find benchmark %s! Use `-mode list` argument to list benchmarks\n", bench_name);
            return;
        }
    } else {
        for (int i = 0; i < tests_len; ++i) {
            if (tests[i].bench != NULL) {
                RunBench(&tests[i]);
            }
        }
    }

    TallyBench();
}

void HandleMode_List()
{
    printf("\nt: has test\tb: has benchmark\nExample: tb:flying_ship (has test and benchmark)\n\n");

    for (int i = 0; i < tests_len; ++i) {
        if (i != 0) {
            printf(", ");
        }
        if (tests[i].test) {
            printf("t");
        }
        if (tests[i].bench) {
            printf("b");
        }
        printf(":");
        assert(tests[i].test || tests[i].bench);
        printf("%s", tests[i].name);
    }
    printf("\n\n");
}

void HandleMode_Help()
{
    printf(
        "Testing utility for Redactor:\n"
        "\t-mode <mode_name> \n"
        "\t-test <test_name> Run specific test\n"
        "<mode_name>\n"
        "\tbench (run benchmarks)\n"
        "\ttest  (default)\n"
        "\tlist  (list tests and benchmarks)\n"
        "\thelp  (print this page)\n"
    );
}

bool ArgWarning(bool check, char *arg) {
    if (check) {
        fprintf(stderr, "Warning: Argument `-%s` ignored\n", arg);
    }  
    return check;
}

bool ArgWarning2(const char *val, char *arg) {
    if (val) {
        fprintf(stderr, "Warning: Argument `-%s %s` ignored\n", arg, val);
    }  
    return val;
}

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "en_US.UTF-8");

    enum {
        Mode_Test,
        Mode_Bench,
        Mode_List,
        Mode_Help
    } mode = Mode_Test;

    const char *last_mode = NULL;
    const char *test_name = NULL;

    IterArgs(arg, argc, argv) {
        if (StrEqualNocase(arg.key, "bench") || StrEqualNocase(arg.key, "test")) {
            if (arg.val == NULL) {
                printf("Choose test/benchmark:\n\t");
                mode = Mode_List;
                break;
            }
            test_name = arg.val;
        } else if (StrEqualNocase(arg.key, "mode")) {
            if      (StrEqualNocase(arg.val, "bench")) { mode = Mode_Bench; }
            else if (StrEqualNocase(arg.val, "test")) { mode = Mode_Test; }
            else if (StrEqualNocase(arg.val, "list")) { mode = Mode_List; }
            else if (StrEqualNocase(arg.val, "help")) { mode = Mode_Help; }
            else    { fprintf(stderr, "Don't know mode `%s`\n", arg.val); return -1; }
            
            ArgWarning2(last_mode, "mode");
            last_mode = arg.val;
        } else if (StrEqualNocase(arg.key, "help")) { // Convenience
            mode = Mode_Help;
            ArgWarning(last_mode, "help");
            last_mode = arg.val;
        }
    }

    switch (mode) {
        case Mode_Test:
            HandleMode_Test(test_name);
            break;
        case Mode_Bench:
            HandleMode_Bench(test_name);
            break;
        case Mode_List:
            HandleMode_List();
            break;
        case Mode_Help:
            HandleMode_Help();
            break;
    }

    return 0;
}
