#include "dbg.h"
#include "buffer.h"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include "utf8.h"
#include "font.h"

struct sstok {
    char *s;
    struct buffer_marker start;
    int color;
};

struct sstoklist {
    struct sstok toks[1024];
    int count;
};

struct synstream {
    struct buffer *buf;
    const char *lval;
    int cval;
    int line, col, lsiz;
};

struct sstok ss_begin(struct synstream *ss);
int ss_peek(struct synstream *ss);
int ss_get(struct synstream *ss);
struct synstream ss_init(struct buffer *buf);

static struct sstok* addtok(struct sstoklist *l, struct sstok tok)
{
    ASSERT(l->count < 1024, "too many tokens");
    l->toks[l->count++] = tok;
    return &l->toks[l->count-1];
}

struct synstream ss_init(struct buffer *buf)
{
    ASSERT(buf->line_count > 0, "empty buf");
    struct synstream result = {0};
    result.buf = buf;
    result.lsiz = buf->lines[0].size;
    result.lval = buf->lines[0].data;
    ss_get(&result);
    return result;
} 

struct sstok ss_begin(struct synstream *ss) 
{
    struct sstok tok = {0};
    tok.start = (struct buffer_marker){ss->line,ss->col};
    return tok;
}

void ss_end(struct synstream *ss, struct sstok *tok)
{
    struct buffer_marker end = {ss->line,ss->col};
    struct buffer_range range = {tok->start, end};
 

    TODO("avoid allocation by iterating over buffer");
    tok->s = buffer_get_range(ss->buf, range);
}

int ss_peek(struct synstream *ss) 
{
    return ss->cval;
}

int ss_get(struct synstream *ss) 
{
    int pval = ss->cval;
    ss->cval = utf8_get(&ss->lval, &ss->lsiz);

    if (ss->line >= ss->buf->line_count) {
        return 0;
    } else if (ss->lsiz <= 0 && ++ss->line < ss->buf->line_count) {
        ss->col = 0;
        ss->lval = ss->buf->lines[ss->line].data;
        ss->lsiz = ss->buf->lines[ss->line].size;
    }

    return pval ? pval : '\n';
}

static void get_tok(struct synstream *ss, struct sstoklist *l)
{
    struct sstok tok = ss_begin(ss);
    while (ss_get(ss)) 
        ;
    ss_end(ss, &tok);
    addtok(l, tok);
}

static SDL_Point draw_highlighted_buffer(SDL_Renderer *renderer, struct font *font, struct buffer *buf, SDL_Rect viewport) 
{
    struct synstream ss = ss_init(buf);
    SDL_Color colortab[] = {
        {250, 220, 190, 255},
        {100, 0, 190, 255},
    };
    int ncolors = sizeof colortab / sizeof colortab[0];
    SDL_Point position = {viewport.x, viewport.y};
    struct sstoklist list = {0};

    while (ss_peek(&ss)) {
        get_tok(&ss, &list);
        while (list.count) {
            struct sstok tok = list.toks[--list.count];

            ASSERT(tok.color >= 0 || tok.color < ncolors, "invalid color");
            SDL_Color color = colortab[tok.color];

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            for (int i = 0; tok.s[i];) {
                char *start = tok.s+i;
                while (tok.s[i] && tok.s[i] != '\n')
                    ++i;
                int c = tok.s[i];
                ASSERT(c == 0 || c == '\n', "character is invalid %x", c);
                tok.s[i] = 0;
                position.x += font_write_text(font, start, position, renderer).x;
                tok.s[i] = c;
                if (c == '\n') 
                    ++i;
                position.x = viewport.x;
                position.y += font_get_height(font);
            }
            free(tok.s);

        }
    }
    return position;
}