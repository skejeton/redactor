#include "../../tests/test.h"
#include "src/Buffer.h"
#include <string.h>

const char *TestString = "Hello world\n"
                         "Привет мир\n"
                         "こんにちは世界\n"
                         "Hej världen";


void TestGetRange()
{
    Buffer buf = Buffer_InitFromString(TestString);
    struct { Range r; const char *exp; } cases[] = {
        {{{0, 0}, Buffer_EndCursor(&buf)}, TestString}, 
        {{{1, 0}, {1, 10}}, "Привет мир"}, 
        {{{1, 0}, {2, 7}}, "Привет мир\nこんにちは世界"}, 
        {{{1, 0}, {2, 7}}, "Привет мир\nこんにちは世界"}, 
        {{{1, 3}, {1, 8}}, "вет м"}, 
        {{{2, 2}, {2, 3}}, "に"}, 
        {{{0, 0}, {0, 0}}, ""}, 
        {{{0, 11}, {1, 0}}, "\n"}, 
    };

    for (int i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        char *t = Buffer_GetStringRange(&buf, cases[i].r);
        Info("Buffer text\n%s\n", t);
        Expect(strcmp(t, cases[i].exp) == 0);
        free(t);
    }

    Buffer_Deinit(&buf);
}

void Test_Buffer_Main()
{
    Info("Testing getting string range");
    TestGetRange();

}
