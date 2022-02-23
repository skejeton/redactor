#include "font.h"
#include <stdbool.h>

static bool getnum(const char *text, const char ** end)
{
    const char *txt = text;
    while (*txt && isdigit(*txt))
        txt++;
    if (text != txt)
        *end = txt;
    return text != txt;
}


static bool getstr(const char *text, const char **end)
{
    if (*text++ == '"') {
        while (*text && *text != '"')
            if (*text++ == '\\')
                text++;
        text++;
        *end = text;
        return 1;
    }
    return 0;
}


static bool getident(const char *text, const char **end)
{
    const char *txt = text;
    if (*txt && !(isalpha(*txt) || *txt == '_'))
        return false;
    while (*txt && (isalnum(*txt) || *txt == '_'))
        txt++;
    if (text != txt)
        *end = txt;
    return text != txt;

}


static bool getcall(const char *text, const char **end)
{
    const char *prev = *end;
    if (getident(text, end) && **end == '(')
        return 1;
    *end = prev;
    return 0;
}


static bool getspc(const char *text, const char **end) 
{
    const char *keytab[] = {
        "NULL", "EOF", "FILE",
    };

    for (int i = 0; i < sizeof(keytab)/sizeof(keytab[0]); i++) {
        int l = strlen(keytab[i]);
        if (strncmp(text, keytab[i], l) == 0 && !(isalnum(text[l]) || text[l] == '_')) {
            *end = text+l;
            return 1;
        }
    }

    return 0;
}


static bool getkw(const char *text, const char **end) 
{
    const char *keytab[] = {
        "auto", "bool", "break", "case", "char",
        "const","continue","default","do",
        "double","else","enum","extern",
        "float","for","goto","if",
        "int","long","register","return",
        "short","signed","sizeof","static",
        "struct","switch","thread_local","typedef","union",
        "unsigned","void","volatile","while"
    };

    for (int i = 0; i < sizeof(keytab)/sizeof(keytab[0]); i++) {
        int l = strlen(keytab[i]);
        if (strncmp(text, keytab[i], l) == 0 && !(isalnum(text[l]) || text[l] == '_')) {
            *end = text+l;
            return 1;
        }
    }

    return 0;
}


static bool getchr(const char *text, const char **end)
{
    if (*text++ == '\'') {
        while (*text && *text != '\'')
            if (*text++ == '\\')
                text++;
        text++;
        *end = text;
        return 1;
    }
    return 0;
}


static bool until_next_line(const char *text, const char **end)
{
    while (*text && *text++ != '\n')
        ;
    *end = text;
    return 1;
}


static int write_line(const char *text, SDL_Point xy, struct font *font, SDL_Renderer *renderer) 
{
    int h = 0, w = 0;
    SDL_SetRenderDrawColor(renderer, 250, 220, 200, 16);
    
    SDL_Point space_glyph_size = font_measure_glyph(' ', font);

    for (int i = 0;*text == ' ';i++,text++) {
        h = space_glyph_size.y;
        if (i % 4 == 0) {
            SDL_RenderFillRect(renderer, &(SDL_Rect){ xy.x+w, xy.y, 1, h });
        }
        w += space_glyph_size.x;
    }
    const char *end = text;
    for (;*text&&*end;text++) {
        const char *prevend = end;
        SDL_Color color;
        if (getnum(text, &end))
            color = (SDL_Color) {201, 134, 0, 255};
        else if (getkw(text, &end))
            color = (SDL_Color) {196, 97, 51, 255};
        else if (getspc(text, &end))
            color = (SDL_Color) {201, 85, 112, 255};
        else if (getcall(text, &end))
            color = (SDL_Color) {245, 90, 66, 255};
        else if (getident(text, &end))
            color = (SDL_Color) {250, 220, 200, 255};
        else if (getstr(text, &end))
            color = (SDL_Color) {95, 135, 35, 255};
        else if (getchr(text, &end))
            color = (SDL_Color) {95, 135, 35, 255};
        else if (strncmp(text, "//", 2) == 0 && until_next_line(text, &end))
            color = (SDL_Color) {82, 76, 60, 255};
        else if (strncmp(text, "#", 1) == 0 && until_next_line(text, &end))
            color = (SDL_Color) {196, 97, 51, 255};
        else
            continue;

        char tail[1024];

        SDL_SetRenderDrawColor(renderer, 250, 220, 200, 255);
        snprintf(tail, 1024, "%.*s", (int)(text-prevend), prevend);
        w += font_write_text(tail, (SDL_Point){xy.x+w, xy.y}, renderer, font).x;

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        snprintf(tail, 1024, "%.*s", (int)(text-prevend), prevend);
        snprintf(tail, 1024, "%.*s", (int)(end-text), text);
        w += font_write_text(tail, (SDL_Point){xy.x+w, xy.y}, renderer, font).x;

        text = end;

    }
    SDL_SetRenderDrawColor(renderer, 250, 220, 200, 255);
    h = font_write_text(end, (SDL_Point){xy.x+w, xy.y}, renderer, font).y;
    return h;
}

