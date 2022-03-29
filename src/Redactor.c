#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#define DieErr(...) do {fprintf(stderr, __VA_ARGS__); exit(-1);} while (0)

struct {
        const char   *file_name;
        FILE         *file_handle;
        char         *file_data;
} 
typedef Redactor;

char* Util_ReadFileStr(FILE *f)
{
        char *s;
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        fseek(f, 0, SEEK_SET);
        s = malloc(size + 1);
        s[fread(s, 1, size, f)] = 0;
        return s;
}

// Checks out the arguments and sets needed values
int Redactor_UseArgs(Redactor *rs, int argc, char *argv[])
{
        if (argc != 2) {
                DieErr("Usage: %s file.txt\n", argv[0]);
        }

        rs->file_name = argv[1];
        rs->file_handle = fopen(rs->file_name, "rw");
        if (!rs->file_handle) {
                DieErr("Fatal: Error opening file %s: %s", rs->file_name, strerror(errno));
        }
        rs->file_data = Util_ReadFileStr(rs->file_handle);
        
}

int Redactor_Main(int argc, char *argv[])
{
        Redactor rs = {0};
        Redactor_UseArgs(&rs, argc, argv);
        printf("%s", rs.file_data);
        free(rs.file_data);
}
