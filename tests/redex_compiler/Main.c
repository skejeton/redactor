#include "../test.h"
#include "src/Redex/Redex.h"

void In_CharsetSynopsis(Redex_Charset set) {
    printf("[");
    if (set.inverted) {
        printf("not ");
    }
    for (int i = 0; i < set.ranges_len; ++i) {
        Redex_CharacterRange range = set.ranges[i];
        if (i != 0) {
            printf(" or ");
        }

        if (range.from == range.to) {
            printf("%lc", range.from);
        } else {
            printf("%lc-%lc", range.from, range.to);
        }
    }
    printf("]");
}

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
            case Redex_SubGroup_Charset: 
                In_CharsetSynopsis(subgroup.charset);
                break;
            case Redex_SubGroup_CharacterClass:
                switch(subgroup.character_class) {
                    case Redex_CharacterClass_Any:
                        printf("[ANY_CHAR]");
                        break;
                }
                break; 
        }
        
        printf("%s", QNT_CHARS[subgroup.quantifier]);
    }
}

void Test_Redex_Compiler_Main()
{
    In_GroupSynopsis(Redex_Compile("ab\\.c.d[a-z0-9]e?(f12)*?3+"));
    printf("\n");
}
