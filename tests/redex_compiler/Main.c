#include "../test.h"
#include "src/Redex/Redex.h"

void In_CharsetSynopsis(Redex_Charset set) {
    printf("\x1b[32m[");
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
    printf("]\x1b[0m");
}

void In_GroupSynopsis(Redex_Group group) 
{
    const char *QNT_CHARS[] = {
        [Redex_Quantifier_None] = "",
        [Redex_Quantifier_All] = "\x1b[31m*\x1b[0m",
        [Redex_Quantifier_Greedy] = "\x1b[32m+\x1b[0m",
        [Redex_Quantifier_Lazy] = "\x1b[34m?\x1b[0m" ,
    };

    for (int i = 0; i < group.subgroups_len; ++i) {
        if (i != 0) {
            printf("\x1b[30m->\x1b[0m");
        }
        
        Redex_SubGroup subgroup = group.subgroups[i];

        switch (subgroup.type) {
            case Redex_SubGroup_Char: 
                printf("%lc", subgroup.ch);
                break;
            case Redex_SubGroup_Group: 
                printf("\x1b[33m(\x1b[0m");
                In_GroupSynopsis(subgroup.group);
                printf("\x1b[33m)\x1b[0m");
                break;
            case Redex_SubGroup_Charset: 
                In_CharsetSynopsis(subgroup.charset);
                break;
            case Redex_SubGroup_CharacterClass:            
                printf("\x1b[32m");
                switch(subgroup.character_class) {
                    case Redex_CharacterClass_Any:
                        printf("[ANY_CHAR]");
                        break;
                }
                printf("\x1b[0m");
                break; 
        }
        
        printf("%s", QNT_CHARS[subgroup.quantifier]);
    }
}

void Test_Redex_Compiler_Main()
{
    Redex_CompiledExpression expr = Redex_Compile("[H][e][l][\\n_\\s][o]");
    In_GroupSynopsis(expr);
    Redex_CompiledExpressionDeinit(&expr);

    printf("\n");
}
