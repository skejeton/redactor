#include "../test.h"
#include "src/Redex.h"
#include <stdbool.h>
#define BIG_ENOUGH 128

static const char *TestString = "Hello world\n"
                                "Привет мир\n"
                                "こんにちは世界\n"
                                "Hej världen";


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

static RedexTest redexTests[] = {
    {
        "Hello",
        {{"Hello", "Hello"}, {"Byebye", NULL}}
    },
    {
        "[a-z]",
        {{"amogus", "a"}, {"Amogus", NULL}}
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
        RedexTest *test = &redexTests[i];
        for (int j = 0; test->sequence[j].text; ++j) {
            Buffer buf = Buffer_InitFromString(test->sequence[j].text);
            Redex_Match match = Redex_GetMatch(&buf, (Cursor){0, 0}, test->redex);
            if (test->sequence[j].match) {
                Expect(match.success);
           
                char *r = Buffer_GetStringRange(&buf, (Range){{0, 0}, match.end});
                Info("Match :: %s", r);
                free(r);
            } else {
                Expect(!match.success);
            }
            Buffer_Deinit(&buf);
        }
    }
}