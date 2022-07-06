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
}
typedef Test;

Test const static tests[] = {
    {"buffer", Test_Buffer_Main},
    {"buffertape", Test_BufferTape_Main},
    {"redex", Test_Redex_Main},
    {"redex_compiler", Test_Redex_Compiler_Main}
};

const int tests_len = sizeof tests / sizeof tests[0];

void RunTest(Test const *t)
{
    printf("%s:\n", t->name);
    t->test();
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
            fprintf(stderr, "Error: Can't find test %s! Add `-mode list` argument to list tests\n", test_name);
            return;
        }
    } else {
        for (int i = 0; i < tests_len; ++i) {
            RunTest(&tests[i]);
        }
    }

    Tally();
}

void HandleMode_TestList()
{
    for (int i = 0; i < tests_len; ++i) {
        if (i != 0) {
            printf(", ");
        }

        printf("%s", tests[i].name);
    }
    printf("\n");
}

void HandleMode_Help()
{
    printf(
        "Testing utility for Redactor:\n"
        "\t-mode <mode_name> \n"
        "\t-test <test_name> Run specific test\n"
        "<mode_name>\n"
        "\ttest (default)\n"
        "\tlist (list test names)\n"
        "\thelp (print this page)\n"
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
        Mode_TestList,
        Mode_Help
    } mode = Mode_Test;

    const char *last_mode = NULL;
    const char *test_name = NULL;

    IterArgs(arg, argc, argv) {
        if (StrEqualNocase(arg.key, "test")) {
            if (arg.val == NULL) {
                printf("Choose test:\n\t");
                mode = Mode_TestList;
                break;
            }
            test_name = arg.val;
        } else if (StrEqualNocase(arg.key, "mode")) {
            if      (StrEqualNocase(arg.val, "test")) { mode = Mode_Test; }
            else if (StrEqualNocase(arg.val, "list")) { mode = Mode_TestList; }
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
        case Mode_TestList:
            ArgWarning(test_name, "test");
            HandleMode_TestList();
            break;
        case Mode_Help:
            ArgWarning(test_name, "test");
            HandleMode_Help();
            break;
    }

    return 0;
}
