#include "Redactor.h"
#include <errno.h>
#if defined(Platform_Is_Linux)
#include <unistd.h>
#include <libgen.h>
#endif

char *Util_Strdup(const char *s)
{
    return strcpy(malloc(strlen(s)+1), s);
}

char *Util_ReadFileStr(FILE *f)
{
    char *s;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    s = malloc(size + 1);
    s[fread(s, 1, size, f)] = 0;
    return s;
}

// Returns malloc'd string
char *Util_GetProgramPath()
{
    const size_t PathMax = 0xFFF;
    const size_t ByteSize = sizeof(char) * PathMax + 1;
    char *path = malloc(ByteSize);
#if defined(Platform_Is_Linux)
        // NOTE: Redundant allocation is needed because
        //       string returned by dirname can be overwritten at any time.
        //       I can try to avoid it but it doesn't matter that much.
        //       The problem is that it //MAY// modify path instead of using a static buffer.
    char *filepath  = malloc(sizeof(char) * PathMax + 1);
        // TODO: Handle path that's more than PathMax.
        //       How would I detect that?
    int written_chars = readlink("/proc/self/exe", filepath, PathMax);
    if (written_chars == -1) {
        DieErr("Fatal: Failed to retrieve process path: %s", strerror(errno));
    }
        // NOTE: Readlink returns the program path including the name,
        //       I don't need that.
    char *dirpath = dirname(filepath);
    int i;
    for (i = 0; i < PathMax && dirpath[i]; ++i)
        path[i] = dirpath[i];
    path[i] = 0;
    free(filepath);
#else
#   error "I can't get program path for this platform"
#endif
    return path;
}

char *Util_ConcatPaths(const char *path_a, const char *path_b)
{
    int la = strlen(path_a), lb = strlen(path_b);
    int len = la + 1 + lb;
    char *s = malloc(len + 1);
    memcpy(s, path_a, la);
#if defined(Platform_Is_Windows) 
    s[la] = '\\';
#elif !defined(Platform_Is_Unknown)
    s[la] = '/';
#else
#   error "I don't know the path separator for this platform"
#endif
    s[len] = 0;
    memcpy(s+la+1, path_b, lb);
    return s;
}

void Util_DieErr(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
#if defined(Platform_Is_Windows)
    char errbuf[1024];
    int MessageBox(void, const char*, const char*, uint32_t);
    vsnprintf(errbuf, 1024, fmt, list);
    MessageBox(NULL, errbuf, "Redactor error", 0x10);
#else
    vfprintf(stderr, fmt, list);
#endif
    va_end(list);
    exit(-1);
}


