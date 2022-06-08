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
    Highlight_Rule_AnyKw,
    Highlight_Rule_Wrapped,
    Highlight_Rule_Redex,
    Highlight_Rule_Lookahead,
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
        struct { 
            const char *data, *tail;
        } rule_lookahead;
    };
}
typedef Highlight_Rule;

static bool In_ProcessWrapped(Redactor *rs, const char *begin, const char *end, const char *escape, BufferTape *tape)
{
    Redex_Match match = Redex_GetMatch(*tape, begin);
    
    // Prevent empty matches
    if (Buffer_CompareCursor(match.end.cursor, tape->cursor) == 0) {
        return false;
    }

    if (match.success) {
        *tape = match.end;
        while (BufferTape_Get(tape)) {
            if ((match = Redex_GetMatch(*tape, end)).success) {
                *tape = match.end;
                break;
            }
            
            if ((match = Redex_GetMatch(*tape, escape)).success && Buffer_CompareCursor(tape->cursor, match.end.cursor) != 0) {
                *tape = match.end;
            } else {
                BufferTape_Next(tape);
            }
        }
        return true;
    } else {
        return false;
    }
}

static bool In_ProcessLookahead(Redactor *rs, const char *data, const char *tail, BufferTape *tape)
{
    Redex_Match match = Redex_GetMatch(*tape, data);

    if (match.success) {
        *tape = match.end;
        return Redex_GetMatch(*tape, tail).success;
    } else {
        return false;
    }
}

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

static bool In_ProcessAnyKw(Redactor *rs, const char **table, BufferTape *tape)
{
    // TODO: Binary search
    for (int i = 0; table[i]; i++) {
        BufferTape copy = *tape;

        // NOTE: This is a quick hack, ideally when I have nested rules I can have keyword derive from identifier
        if (In_ProcessRedex(rs, table[i], &copy) && !isalnum(BufferTape_Get(&copy)) && BufferTape_Get(&copy) != '_') {
            *tape = copy;
            return true;
        }
    }
    return false;
}

void Highlight_HighlightBuffer(Redactor *rs, BufferDrawSegments *segments)
{
    BufferDraw_InvalidateSegments(segments);

    Highlight_Rule rules[32];
    int rule_count = 0;

    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_AnyKw, Redactor_Color_Keyword, {.rule_anykw = keytab}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_AnyKw, Redactor_Color_Literal, {.rule_anykw = symtab}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Lookahead, Redactor_Color_Call, {.rule_lookahead = {"[a-zA-Z_]+[a-zA-Z_0-9]*", "[ ]*\\("}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Lookahead, Redactor_Color_Keyword, {.rule_lookahead = {"[a-zA-Z_]+[a-zA-Z_0-9]*", "[*( ]*[a-zA-Z_]+[a-zA-Z_0-9]*"}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_Fore, {.rule_redex= "[a-zA-Z_]+[a-zA-Z_0-9]*"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_Literal, {.rule_redex= "[0-9]+"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_String, {.rule_wrapped = {"\"", "\"", "\\\\."}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_String, {.rule_wrapped = {"\'", "\'", "\\\\."}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Faded, {.rule_wrapped = {"/\\*", "\\*/", ""}}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Redex, Redactor_Color_Faded, {.rule_redex = "//[^\\n]*"}};
    rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Faded, {.rule_wrapped = {"#", "\n", ""}}};

    BufferTape tape = BufferTape_Init(&rs->file_buffer);
    BufferTape newTape;
    while (BufferTape_Get(&tape)) {
        SDL_Color color = Redactor_Color_Fore;

        bool match = false;
        for (int i = 0; i < rule_count; ++i)  {
            newTape = tape;
            Highlight_Rule *rule = &rules[i];
            switch (rule->rule_type) {
            case Highlight_Rule_Redex:
                match = In_ProcessRedex(rs, rule->rule_redex, &newTape);
                break;
            case Highlight_Rule_Lookahead:
                match = In_ProcessLookahead(rs, rule->rule_lookahead.data, rule->rule_lookahead.tail, &newTape);
                break;
            case Highlight_Rule_Wrapped:
                match = In_ProcessWrapped(rs, rule->rule_wrapped.begin, rule->rule_wrapped.end, rule->rule_wrapped.slash, &newTape);
                break;
            case Highlight_Rule_AnyKw:
                match = In_ProcessAnyKw(rs, rule->rule_anykw, &newTape);
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
            BufferDraw_InsertSegment(segments, tape.cursor.line, tape.cursor.column, BufferTape_GetSubstringMemoryOffset(&tape), Redactor_Color_Fore);
            BufferDraw_InsertSegment(segments, newTape.cursor.line, newTape.cursor.column, BufferTape_GetSubstringMemoryOffset(&newTape), color);
            tape = newTape;
        } else {
            BufferTape_Next(&tape);
        }
    }
    BufferDraw_InsertSegment(segments, tape.cursor.line, tape.cursor.column, BufferTape_GetSubstringMemoryOffset(&tape), Redactor_Color_Fore);
}

