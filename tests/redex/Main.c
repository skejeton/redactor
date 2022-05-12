#include "../test.h"
#include "src/Redex.h"
#include <string.h>
#include <stdbool.h>
#define BIG_ENOUGH 128

struct RedexTestClause {
    // NOTE: NULL value will terminate the sequence
    const char *text;
    // NOTE: Set to NULL if the clause should fail
    const char *match;
}
typedef RedexTestClause;

struct RedexTest {
    const char *redex;
    RedexTestClause sequence[BIG_ENOUGH];
}
typedef RedexTest;

static const RedexTest redexTests[] = {
    {
        "Hello",
        {{"Hello", "Hello"}, {"Byebye", NULL}}
    },
    {
        "\n",
        {{"\n", "\n"}}
    },
    {
        "te[%n_%s]st",
        {{"te\nst", "te\nst"}, {"te_st", "te_st"}, {"te st", "te st"}}
    },
    {
        "([a-z][0-9])+",
        {{"a1b2c3", "a1b2c3"}, {"a1b2cc", "a1b2"}, {"aa", NULL}}
    },
        {
        "%|(([a-z][0-9])+%|)+",
        {{"|a1b2|c3|", "|a1b2|c3|"}, {"|a1|b2cc|", "|a1|"}, {"|aa", NULL}, {"|", NULL}, {"aa", NULL}}
    },
    {
        "[a-z]",
        {{"amogus", "a"}, {"Amogus", NULL}}
    },  
    {
        "#[^%n]*",
        {{"#include <stdio.h>\ntest", "#include <stdio.h>"}, {"#\ntest", "#"}, {"\ntest", NULL}}
    },  
    {
        "[^a-z_]",
        {{"_", NULL}, {"e", NULL}, {"1", "1"}}
    },  
    {
        "[a-z]+",
        {{"anyword", "anyword"}, {"love you", "love"}, {" ", NULL}}
    },  
    {
        "[а-яА-Я]*",
        {{"Кириллица", "Кириллица"}, {"Or", ""}, {"", ""}}
    },
    {
        "[z-a]",
        {{"amogus", NULL}, {"Amogus", NULL}}
    },  
    {
        "[%]]",
        {{"]", "]"}, {"%", NULL}}
    },  
    {
        "[H][e][l][lこx][o]",
        {{"Helこo", "Helこo"}, {"Helxo", "Helxo"}, {"Helno", NULL}}
    }
};


void Test_Redex_Main() 
{
    for (int i = 0; i < sizeof(redexTests) / sizeof(redexTests[0]); ++i) {
        const RedexTest *test = &redexTests[i];
        for (int j = 0; test->sequence[j].text; ++j) {
            Buffer buf = Buffer_InitFromString(test->sequence[j].text);
            Redex_Match match = Redex_GetMatch(&buf, (Cursor){0, 0}, test->redex);

            if (test->sequence[j].match) {
                Expect(match.success);
                char *r = Buffer_GetStringRange(&buf, (Range){{0, 0}, match.end});
                Expect(strcmp(r, test->sequence[j].match) == 0);
                Info("Match :: '%s'", r);
                free(r);
            } else {
                Expect(!match.success);
            }
            Buffer_Deinit(&buf);
        }
    }
}