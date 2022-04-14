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
