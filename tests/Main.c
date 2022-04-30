#include "test.h"

void Test_Buffer_Main();

int total_asserts, total_passed;

int main() 
{
    RunTest(Test_Buffer_Main());
    Tally();
}
