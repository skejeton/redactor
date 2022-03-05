#include <stdio.h>
#include "test.h"
int total_asserts = 0;
int total_passed = 0;

void test_utf8();

int main()
{
    run_test(test_utf8());

    tally();
}