#include "Redactor.h"
#include "BufferDraw.h"
#include "Utf8.h"
#include "Redex.h"

const char *keytab[] = {
    "auto", "bool", "break", "case", "char",
    "const","continue","default","double",
    "do","else","enum","extern",
    "float","for","goto","if",
    "int","long","register","return",
    "short","signed","sizeof","static",
    "struct","switch","thread_local","typedef","union",
    "unsigned","void","volatile","while", NULL
};

const char *symtab[] = {
    "NULL", "false", "true", NULL
};

enum {
    Highlight_Rule_AnyChar,
    Highlight_Rule_AnyKw,
    Highlight_Rule_Wrapped,
    Highlight_Rule_Redex,
};

struct {
    int rule_type;
    SDL_Color color;
    union {
        const char **rule_anykw;
        struct { 
            const char *begin, *end, *slash;
        } rule_wrapped;
        const char *rule_redex;
    };
}
typedef Highlight_Rule;

static bool In_ProcessRedex(Redactor *rs, const char *redex, BufferTape *tape)
{
    Redex_Match match = Redex_GetMatch(*tape, redex);

    if (match.success) {
        *tape = match.end;
        return true;
    } else {
        return false;
    }
}

void Highlight_HighlightBuffer(Redactor *rs, BufferDrawSegments *segments)
{
    BufferDraw_InvalidateSegments(segments);

    Highlight_Rule rules[32];
    int rule_count = 0;
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_White, {.rule_redex= "[a-zA-Z_]+[a-zA-Z_0-9]*"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_Yellow, {.rule_redex= "[0-9]+"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_AnyKw, Redactor_Color_Green, {.rule_anykw = keytab}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_Pinkish, {.rule_redex = "\"[^\"]*\"?"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Pinkish, {.rule_wrapped = {"\'", "\'", "\\"}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Gray, {.rule_wrapped = {"/*", "*/", ""}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_Gray, {.rule_redex = "//[^\\n]*"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Gray, {.rule_wrapped = {"#", "\n", ""}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_AnyKw, Redactor_Color_Pinkish, {.rule_anykw = symtab}};

    BufferTape tape = BufferTape_Init(&rs->file_buffer);
    BufferTape newTape;
    while (BufferTape_Get(&tape)) {
        newTape = tape;
        SDL_Color color = Redactor_Color_White;

        bool match = false;
        for (int i = 0; i < rule_count; ++i)  {
            Highlight_Rule *rule = &rules[i];
            switch (rule->rule_type) {
            case Highlight_Rule_Redex:
                match = In_ProcessRedex(rs, rule->rule_redex, &newTape);
                break;
            default: 
                break;
            }

            if (match) {
                color = rule->color;
                break;
            } 
        }

        if (match) {
            BufferDraw_InsertSegment(segments, tape.cursor.line, tape.cursor.column, BufferTape_GetSubstringMemoryOffset(&tape), Redactor_Color_White);
            BufferDraw_InsertSegment(segments, newTape.cursor.line, newTape.cursor.column, BufferTape_GetSubstringMemoryOffset(&newTape), color);
            tape = newTape;
        } else {
            BufferTape_Next(&tape);
        }
    }
    BufferDraw_InsertSegment(segments, tape.cursor.line, tape.cursor.column, BufferTape_GetSubstringMemoryOffset(&tape), Redactor_Color_White);
}

