#include "dbg.h"
#include <stdio.h>
#include <stdarg.h>

void todo__(const char *file, int line, const char *function, const char *fmt, ...)
{
    fprintf(stderr, "\x1b[34m((TODO)) %s:%d %s:\x1b[0m ", file, line, function);
    va_list list;
    va_start(list, fmt);
    vfprintf(stderr, fmt, list);
    va_end(list);
    fprintf(stderr, "\n");
}
