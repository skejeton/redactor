#include "HighlightSets.h"
#include "Colors.h"

#define R(x) Highlight_Rule_##x
#define C(x) Redactor_Color_##x
#define AnyKw(x) {.rule_anykw = (x)}
#define Lookahead(r, a) {.rule_lookahead = {(r), (a)}}
#define Redex(r) {.rule_redex = (r)}
#define Wrapped(sta, end, esc) {.rule_wrapped = {(sta), (end), (esc)}}

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
    {R(AnyKw),      C(Keyword), AnyKw(C_keytab)},
    {R(AnyKw),      C(Literal), AnyKw(C_symtab)},
    {R(Lookahead),  C(Call),    Lookahead("[a-zA-Z_]+[a-zA-Z_0-9]*", "[ ]*\\(")},
    {R(Lookahead),  C(Keyword), Lookahead("[a-zA-Z_]+[a-zA-Z_0-9]*", "[*( ]*[a-zA-Z_]+[a-zA-Z_0-9]*")},
    {R(Redex),      C(Fore),    Redex("[a-zA-Z_]+[a-zA-Z_0-9]*")},
    {R(Redex),      C(Literal), Redex("[0-9]+")},
    {R(Wrapped),    C(String),  Wrapped("\"", "\"", "\\\\.")},
    {R(Wrapped),    C(String),  Wrapped("\'", "\'", "\\\\.")},
    {R(Wrapped),    C(Faded),   Wrapped("/\\*", "\\*/", "")},
    {R(Redex),      C(Faded),   Redex("//[^\\n]*")},
    {R(Wrapped),    C(Faded),   Wrapped("#", "\n", "")}
};

const Highlight_Set HighlightSets_C = {C_rules, sizeof C_rules / sizeof 0[C_rules]};
