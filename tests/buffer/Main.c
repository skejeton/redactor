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
#if 0
        Info("Buffer text\n%s\n", t);
#endif
        Expect(strcmp(t, cases[i].exp) == 0);
        free(t);
    }

    Buffer_Deinit(&buf);
}

void TestInsert()
{
    struct { Cursor at; const char *init, *put, *exp;} cases[] = {
        {{0, 0}, "", "Привет мир", "Привет мир"}, 
        {{0, 2}, "Tet", "x", "Text"}, 
        {{0, 3}, "Привет мир", "Test\n", "ПриTest\nвет мир"}, 
        {{0, 3}, "Привет мир", "Test\n\n", "ПриTest\n\nвет мир"}, 
        {{1, 0}, "Привет\nмир", "Test\n", "Привет\nTest\nмир"}, 
        {{0, 6}, "Привет\nмир", "Test\n", "ПриветTest\n\nмир"}, 
        {{0, 7}, "こんにちは世界", "\n", "こんにちは世界\n"}, 
        {{0, 0}, "こんにちは世界", "\n", "\nこんにちは世界"}, 
    };

    for (int i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        Buffer buf = Buffer_InitFromString(cases[i].init);
        Buffer_InsertUTF8(&buf, cases[i].at, cases[i].put);
        char *t = Buffer_GetStringRange(&buf, (Range){{0, 0}, Buffer_EndCursor(&buf)});
        Expect(strcmp(t, cases[i].exp) == 0);
        free(t);
        Buffer_Deinit(&buf);
    }
}

void Test_Buffer_Main()
{
    Info("Test(GetRange)");
    TestGetRange();
    Info("Test(Insert)");
    TestInsert();
}
