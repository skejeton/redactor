#include "test.h"
#include <locale.h>

void Test_Buffer_Main();
void Test_BufferTape_Main();
void Test_Redex_Main();
void Test_Redex_Compiler_Main();

int total_asserts, total_passed;

int main() 
{
    setlocale(LC_ALL, "en_US.UTF-8");

    RunTest(Test_Buffer_Main());
    RunTest(Test_BufferTape_Main());
    RunTest(Test_Redex_Compiler_Main());
    RunTest(Test_Redex_Main());
    Tally();
}
