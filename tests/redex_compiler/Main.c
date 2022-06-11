#include "../test.h"
#include "src/Redex/Redex.h"

void In_GroupSynopsis(Redex_Group group) 
{
    const char *QNT_CHARS[] = {
        [Redex_Quantifier_None] = "",
        [Redex_Quantifier_All] = "*",
        [Redex_Quantifier_Greedy] = "+",
        [Redex_Quantifier_Lazy] = "?" ,
    };

    for (int i = 0; i < group.subgroups_len; ++i) {
        if (i != 0) {
            printf("->");
        }
        
        Redex_SubGroup subgroup = group.subgroups[i];

        switch (subgroup.type) {
        case Redex_SubGroup_Char: 
            printf("%lc", subgroup.ch);
            break;
        case Redex_SubGroup_Group: 
            printf("(");
            In_GroupSynopsis(subgroup.group);
            printf(")");
            break;
        }
        
        printf("%s", QNT_CHARS[subgroup.quantifier]);
    }
}

void Test_Redex_Compiler_Main()
{
    In_GroupSynopsis(Redex_Compile("abcde?(f12)*?3+"));
    printf("\n");
}
