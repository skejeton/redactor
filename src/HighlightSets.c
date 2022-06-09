#include "HighlightSets.h"
#include "Redactor.h"

#define Ruleset(x) x, sizeof(x)/sizeof(0[x])

static const char *C_keytab[] = {
    "auto", "bool", "break", "case", "char",
    "const","continue","default","double",
    "do","else","enum","extern",
    "float","for","goto","if",
    "int","long","register","return",
    "short","signed","sizeof","static",
    "struct","switch","thread_local","typedef","union",
    "unsigned","void","volatile","while", NULL
};

static const char *C_symtab[] = {
    "NULL", "false", "true", NULL
};

static Highlight_Rule C_rules[] = {
    {Highlight_Rule_AnyKw, Redactor_Color_Keyword, {.rule_anykw = C_keytab}},
    {Highlight_Rule_AnyKw, Redactor_Color_Literal, {.rule_anykw = C_symtab}},
    {Highlight_Rule_Lookahead, Redactor_Color_Call, {.rule_lookahead = {"[a-zA-Z_]+[a-zA-Z_0-9]*", "[ ]*\\("}}},
    {Highlight_Rule_Lookahead, Redactor_Color_Keyword, {.rule_lookahead = {"[a-zA-Z_]+[a-zA-Z_0-9]*", "[*( ]*[a-zA-Z_]+[a-zA-Z_0-9]*"}}},
    {Highlight_Rule_Redex, Redactor_Color_Fore, {.rule_redex= "[a-zA-Z_]+[a-zA-Z_0-9]*"}},
    {Highlight_Rule_Redex, Redactor_Color_Literal, {.rule_redex= "[0-9]+"}},
    {Highlight_Rule_Wrapped, Redactor_Color_String, {.rule_wrapped = {"\"", "\"", "\\\\."}}},
    {Highlight_Rule_Wrapped, Redactor_Color_String, {.rule_wrapped = {"\'", "\'", "\\\\."}}},
    {Highlight_Rule_Wrapped, Redactor_Color_Faded, {.rule_wrapped = {"/\\*", "\\*/", ""}}},
    {Highlight_Rule_Redex, Redactor_Color_Faded, {.rule_redex = "//[^\\n]*"}},
    {Highlight_Rule_Wrapped, Redactor_Color_Faded, {.rule_wrapped = {"#", "\n", ""}}}
};

const Highlight_Set HighlightSets_C = {C_rules, sizeof C_rules / sizeof 0[C_rules]};
