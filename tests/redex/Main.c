#include "../test.h"
#include "src/Redex.h"
#include <stdbool.h>

static const char *TestString = "Hello world\n"
                                "Привет мир\n"
                                "こんにちは世界\n"
                                "Hej världen";


void Test_Redex_Main() 
{
    Buffer buf = Buffer_InitFromString("Helこo");
    Redex_Match match = Redex_GetMatch(&buf, (Cursor){0, 0}, "[H][e][l][lこx][o]");
    Expect(match.success);
    if (match.success) {
        char *r = Buffer_GetStringRange(&buf, (Range){{0, 0}, match.end});
        Info("%s\n", r);
        free(r);
    }
    Buffer_Deinit(&buf);
}