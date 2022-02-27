#include "util.h"
#include <stdio.h>

void *malloc(size_t c);
size_t strlen(const char *);

char* util_read_whole_file(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (f == NULL)
        return NULL;
    fseek(f, 0, SEEK_END);
    size_t fsz = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    char* input = (char*)malloc((fsz + 1) * sizeof(char));
    input[fsz] = 0;
    fread(input, sizeof(char), fsz, f);
    fclose(f);
    return input;
}


void util_write_whole_file(const char* path, const char *contents)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL)
        return;

    fwrite(contents, strlen(contents), 1, f);
    fclose(f);
}