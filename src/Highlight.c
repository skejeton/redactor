#include "Redactor.h"
#include "Unicode.h"

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

static const char* FindKw(const char *kw)
{
        for (int i = 0; keytab[i]; ++i) {
                if (strncmp(kw, keytab[i], strlen(keytab[i])) == 0) {
                        return keytab[i];
                } 
        }
        return NULL;
}

enum {
        Highlight_Rule_AnyChar,
        Highlight_Rule_Wrapped,
};

struct {
        int rule_type;
        SDL_Color color;
        union {
                const char *rule_anychar;
                struct { 
                        const char *begin, *end, *slash;
                } rule_wrapped;
        };
}
typedef Highlight_Rule;




bool Highlight_Process_AnyChar(Redactor *rs, const char *charset, int *line_no, Line *line)
{
        int n = 0, c;
        char *prev = line->text;
        while ((c = Uni_Utf8_NextVeryBad(&line->text)) && Uni_Utf8_Strchr(charset, c)) {
                prev = line->text;
                n++;
        }
        line->text = prev;

        return n > 0;
}

bool Highlight_Process_Wrapped(Redactor *rs, const char *begin, const char *end, const char *slash, int *line_no, Line *line)
{
        int sb = strlen(begin), se = strlen(end), ss = strlen(slash);

        if (strncmp(line->text, begin, sb) == 0) {
                line->text += sb;
 
                while (*line->text && strncmp(line->text, end, se) != 0) {
                        if (strncmp(line->text, slash, ss) == 0) {
                                line->text += ss;
                        }
                        if (*line->text) {
                                line->text++;
                        }
                }
                if (*line->text) {
                        line->text += se;
                }
                return true;
        } else {
                return false;
        }
}

void Highlight_DrawHighlightedBuffer(Redactor *rs)
{
        SDL_Point position = {rs->render_scroll.x, rs->render_scroll.y};
        int height = rs->render_font_chunks[0]->glyphs[' '].h;

        Highlight_Rule rules[32];
        int rule_count = 0;
        rules[rule_count++] = (Highlight_Rule){Highlight_Rule_AnyChar, Redactor_Color_Yellow, "0123456789"};
        rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Pinkish, {.rule_wrapped = {"\"", "\"", "\\"}}};
        rules[rule_count++] = (Highlight_Rule){Highlight_Rule_Wrapped, Redactor_Color_Gray, {.rule_wrapped = {"//", "\n", ""}}};
        

        int line_no = 0;

        while (line_no < rs->file_buffer.lines_len) {
                int col = 0;
                Line line = rs->file_buffer.lines[line_no];
                while (*line.text) {
                        char *start = line.text;
                        bool match = false;
                        SDL_Color color = Redactor_Color_White;

                        for (int i = 0; i < rule_count; ++i)  {
                                Highlight_Rule *rule = &rules[i];
                                switch (rule->rule_type) {
                                case Highlight_Rule_AnyChar:
                                        match = Highlight_Process_AnyChar(rs, rule->rule_anychar, &line_no, &line);
                                        break;
                                case Highlight_Rule_Wrapped:
                                        match = Highlight_Process_Wrapped(rs, rule->rule_wrapped.begin, rule->rule_wrapped.end, rule->rule_wrapped.slash, &line_no, &line);
                                        break;
                                }

                                if (match) {
                                        color = rule->color;
                                        break;
                                }
                        }

                        if (!match) {
                                line.text = start;
                                Uni_Utf8_NextVeryBad(&line.text);
                        }

                        int tc = *line.text;
                        *line.text = 0;
                        SDL_Point delta = Redactor_DrawText(rs, color, start, position.x, position.y, col);
                        // FIXME: Slow, and pretty much a hack anyway
                        int c;
                        while (c = Uni_Utf8_NextVeryBad(&start)) {
                                if (c == '\t') {
                                        col += (8 - (col % 8));
                                } else {
                                        col += 1;
                                }
                        }
                        height = delta.y;
                        position.x += delta.x;
                        *line.text = tc;
                }

                position.y += height;
                position.x = rs->render_scroll.x;
                line_no++;
        }
}

#if 0
void Highlight_DrawHighlightedBuffer(Redactor *rs)
{
        SDL_Point position = {rs->render_scroll.x, rs->render_scroll.y};
        int height = rs->render_font_chunks[0]->glyphs[' '].h;
        bool comment = false;

        for (int i = 0; i < rs->file_buffer.lines_len; ++i) {
                Line line = rs->file_buffer.lines[i];
                int c;
                int col = 0;

                while (*line.text) {
                        const char *tok;
                        char *startl = line.text;

                        if (comment) {
                                while (*line.text) {
                                        if (strncmp(line.text, "*/", 2) == 0) {
                                                comment = false; 
                                                SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_Gray, "*/", position.x, position.y);
                                                height = delta.y;
                                                position.x += delta.x;
                                                line.text += 2;

                                                break;
                                        }
                                        int c = Uni_Utf8_NextVeryBad(&line.text);
                                        int tc = *line.text;
                                        *line.text = 0;

                                        if (c == '\t') {
                                                position.x += rs->render_font_chunks[0]->glyphs[' '].w * (8 - (col % 8));
                                                col += (8 - (col % 8));
                                        } else {
                                                SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_Gray, startl, position.x, position.y);
                                                height = delta.y;
                                                position.x += delta.x;
                                                col += 1;
                                        }

                                        *line.text = tc;
                                        startl = line.text;
                                }
                        } else if (*line.text == '#') {
                                line.text++;
                                while (*line.text && isalpha(*line.text)) {
                                        line.text++;
                                        col += 1;
                                }

                                int tc = *line.text;
                                *line.text = 0;
                                SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_Pink, startl, position.x, position.y);
                                height = delta.y;
                                position.x += delta.x;
                                *line.text = tc;
                        } else if (*line.text == '\"') {
                                int spx = position.x;
                                int skip = 0;
                                col += 1;
                                while (Uni_Utf8_NextVeryBad(&line.text) && (*line.text != '\"' || skip)) {
                                        if (*line.text == '\\')  {
                                                skip = 1;
                                                Uni_Utf8_NextVeryBad(&line.text);
                                                col += 2;
                                        } else if (*line.text == '\t') {
                                                position.x += rs->render_font_chunks[0]->glyphs[' '].w * (8 - (col % 8));
                                                col += (8 - (col % 8));
                                        } else {
                                                col += 1;
                                        }
                                        skip = 0;
                                }
                                if (*line.text) {
                                        col += 1;
                                        *line.text++;
                                }
                                int tc = *line.text;
                                *line.text = 0;
                                SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_Pinkish, startl, spx, position.y);
                                height = delta.y;
                                position.x += delta.x;
                                *line.text = tc;
                        } else if (*line.text == '<') {
                                int spx = position.x;
                                int skip = 0;
                                col += 1;
                                while (Uni_Utf8_NextVeryBad(&line.text) && (*line.text != '>' || skip)) {
                                        if (*line.text == '\\')  {
                                                skip = 1;
                                                Uni_Utf8_NextVeryBad(&line.text);
                                                col += 2;
                                        } else if (*line.text == '\t') {
                                                position.x += rs->render_font_chunks[0]->glyphs[' '].w * (8 - (col % 8));
                                                col += (8 - (col % 8));
                                        } else {
                                                col += 1;
                                        }
                                        skip = 0;
                                }
                                if (*line.text) {
                                        col += 1;
                                        *line.text++;
                                }
                                int tc = *line.text;
                                *line.text = 0;
                                SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_Pinkish, startl, spx, position.y);
                                height = delta.y;
                                position.x += delta.x;
                                *line.text = tc;
                        } else if (strncmp(line.text, "//", 2) == 0) {
                                SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_Gray, startl, position.x, position.y);
                                break;
                        } else if (strncmp(line.text, "/*", 2) == 0) {
                                comment = true;
                                SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_Gray, "/*", position.x, position.y);
                                height = delta.y;
                                position.x += delta.x;
                                line.text += 2;
                        } else if (isalpha(*line.text)) {
                                if ((tok = FindKw(line.text))) {
                                        line.text += strlen(tok);
                                        col += strlen(tok);
                                        SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_Green, tok, position.x, position.y);
                                        height = delta.y;
                                        position.x += delta.x;
                                } else {
                                        while (isalpha(*line.text)) {
                                                line.text++;
                                                col++;
                                        }
                                        int tc = *line.text;
                                        *line.text = 0;
                                        SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_White, startl, position.x, position.y);
                                        height = delta.y;
                                        position.x += delta.x;
                                        *line.text = tc;
                                        
                                }
                        } else if ((c = Uni_Utf8_NextVeryBad(&line.text))) {
                                int tc = *line.text;
                                *line.text = 0;

                                if (c == '\t') {
                                        position.x += rs->render_font_chunks[0]->glyphs[' '].w * (8 - (col % 8));
                                        col += (8 - (col % 8));
                                } else if (isdigit(c)) {
                                        SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_Yellow, startl, position.x, position.y);
                                        height = delta.y;
                                        position.x += delta.x;
                                        col += 1;
                                } else {
                                        SDL_Point delta = Redactor_DrawText(rs, Redactor_Color_White, startl, position.x, position.y);
                                        height = delta.y;
                                        position.x += delta.x;
                                        col += 1;
                                }
                                *line.text = tc;
                        } else {
                                break;
                        }
                }

                position.x = 0;
                position.y += height;
        }
}
#endif
