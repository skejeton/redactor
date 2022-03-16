#include "dbg.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define VAERR(fmt) do {va_list list; va_start(list, fmt); vfprintf(stderr, fmt, list); va_end(list); } while(0)

void todo__(const char *file, int line, const char *function, const char *fmt, ...)
{
    fprintf(stderr, "\x1b[34;1m((TODO)) %s:%d %s:\x1b[0m ", file, line, function);
    VAERR(fmt);
    fprintf(stderr, "\n");
}

int assert__(const char *cond, _Bool x, const char *fmt, ...)
{
    if (!x) {
        fprintf(stderr, "\x1b[31;1massertion failed\x1b[0m: %s \x1b[37m// ", cond);
        VAERR(fmt);
        fprintf(stderr, "\x1b[0m\n");
        return 1;
    }
    return 0;
}
