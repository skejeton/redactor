#include "HighlightSets.h"
#include "Colors.h"
#include "Redex/Redex.h"

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

static HighlightSets_Rule C_rules[] = {
    {R(AnyKw),      C(Keyword), AnyKw(C_keytab)},
    {R(AnyKw),      C(Literal), AnyKw(C_symtab)},
    {R(Lookahead),  C(Call),    Lookahead("[a-zA-Z_][a-zA-Z_0-9]*", "[ ]*\\(")},
    {R(Lookahead),  C(Keyword), Lookahead("[a-zA-Z_][a-zA-Z_0-9]*", "[*( ]*[a-zA-Z_][a-zA-Z_0-9]*")},
    {R(Redex),      C(Fore),    Redex("[a-zA-Z_][a-zA-Z_0-9]*")},
    {R(Redex),      C(Literal), Redex("[0-9][0-9]*")},
    {R(Wrapped),    C(String),  Wrapped("\"", "\"", "\\\\.")},
    {R(Wrapped),    C(String),  Wrapped("\'", "\'", "\\\\.")},
    {R(Wrapped),    C(Faded),   Wrapped("/\\*", "\\*/", "")},
    {R(Redex),      C(Faded),   Redex("//[^\\n]*")},
    {R(Wrapped),    C(Faded),   Wrapped("#", "\n", "")}
};

const HighlightSets_Set HighlightSets_C = {C_rules, sizeof C_rules / sizeof 0[C_rules]};

Highlight_Set HighlightSets_Compile(const HighlightSets_Set *set)
{
    Highlight_Set out_set = {0};
    out_set.rules = calloc(sizeof(Highlight_Rule), set->rules_len);

    for (int i = 0; i < set->rules_len; ++i) {
        HighlightSets_Rule *rule = &set->rules[i];
        Highlight_Rule *out_rule = &out_set.rules[out_set.rules_len++];
        out_rule->color = rule->color;
        out_rule->rule_type = rule->rule_type;

        switch (rule->rule_type) {
            case Highlight_Rule_AnyKw: {
                int len = 0;
                while (rule->rule_anykw[len]) {
                    len++;
                }

                out_rule->rule_anykw.exprs = malloc(sizeof(Redex_CompiledExpression)*len);
                for (int j = 0; j < len; ++j) {
                    out_rule->rule_anykw.exprs[out_rule->rule_anykw.exprs_len++] = Redex_Compile(rule->rule_anykw[j]);
                }
            } break;
            case Highlight_Rule_Lookahead: {
                out_rule->rule_lookahead.data = Redex_Compile(rule->rule_lookahead.data);
                out_rule->rule_lookahead.tail = Redex_Compile(rule->rule_lookahead.tail);
            } break;
            case Highlight_Rule_Redex: {
                out_rule->rule_redex = Redex_Compile(rule->rule_redex);
            } break;
            case Highlight_Rule_Wrapped: {
                out_rule->rule_wrapped.begin = Redex_Compile(rule->rule_wrapped.begin);
                out_rule->rule_wrapped.end = Redex_Compile(rule->rule_wrapped.end);
                out_rule->rule_wrapped.slash = Redex_Compile(rule->rule_wrapped.slash);
            } break;
        }
    }
    return out_set;
}