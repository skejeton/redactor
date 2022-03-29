#include <stdio.h>

struct {
        const char *message;
} 
typedef Redactor;

void Redactor_PrintMessage(Redactor *rs)
{
        printf("%s", rs->message);
}

int Redactor_Main(int argc, char *argv[])
{
        Redactor rs = { "Redactor main\n" };
        Redactor_PrintMessage(&rs);
}
