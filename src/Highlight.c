#include "Redactor.h"
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

static SDL_Point In_DrawTapeDifference(Redactor *rs, SDL_Point at, BufferTape start, BufferTape end, SDL_Color color) 
{
    int tmp = *end.line.text;
    SDL_Point newPos;
    *end.line.text = 0;
    while (start.cursor.line <= end.cursor.line) {
        newPos = Redactor_DrawText(rs, color, start.line.text, 0, at.x, at.y, start.cursor.column);
        if (start.cursor.line < end.cursor.line) {
        at.x = 0;
        at.y += rs->render_font_height;
        }
        start.cursor.column = 0;
        start.cursor.line += 1;
        start.line = start.buffer->lines[start.cursor.line];
    }
    at.x = newPos.x;
    *end.line.text = tmp;

    return at;
}

void Highlight_DrawHighlightedBuffer(Redactor *rs)
{
    SDL_Point position = {rs->render_scroll.x, rs->render_scroll.y};
    int height = rs->render_font_height;

    Highlight_Rule rules[32];
    int rule_count = 0;
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_AnyKw, Redactor_Color_Green, {.rule_anykw = keytab}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_White, {.rule_redex= "[a-zA-Z_]+[a-zA-Z_0-9]*"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_Yellow, {.rule_redex= "[0-9]+"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_Pinkish, {.rule_redex = "\"[^\"]*\"?"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Pinkish, {.rule_wrapped = {"\'", "\'", "\\"}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Gray, {.rule_wrapped = {"/*", "*/", ""}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_Gray, {.rule_redex = "//[^\\n]*"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Gray, {.rule_wrapped = {"#", "\n", ""}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_AnyKw, Redactor_Color_Pinkish, {.rule_anykw = symtab}};
    
    BufferTape tape = BufferTape_Init(&rs->file_buffer);
    BufferTape newTape;
    BufferTape tapeStart = tape;

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
            position = In_DrawTapeDifference(rs, position, tapeStart, tape, Redactor_Color_White);
            tapeStart = tape;
            tape = newTape;
            position = In_DrawTapeDifference(rs, position, tapeStart, tape, color);
            tapeStart = newTape;
        } else {
            BufferTape_Next(&tape);
        }
    }
    position = In_DrawTapeDifference(rs, position, tapeStart, tape, Redactor_Color_White);
}

