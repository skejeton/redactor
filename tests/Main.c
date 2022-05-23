#include "test.h"

void Test_Buffer_Main();
void Test_BufferTape_Main();
void Test_Redex_Main();

int total_asserts, total_passed;

int main() 
{
    RunTest(Test_Buffer_Main());
    RunTest(Test_BufferTape_Main());
    RunTest(Test_Redex_Main());
    Tally();
}
