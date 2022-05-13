#ifndef R_UTIL_H
#define R_UTIL_H

#include <stdio.h>

// -- Util
char *Util_Strdup(const char *s);
char *Util_ReadFileStr(FILE *f);
char *Util_GetProgramPath();
char *Util_ConcatPaths(const char *path_a, const char *path_b);
void Util_DieErr(const char *fmt, ...);

#endif