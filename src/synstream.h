#include <SDL2/SDL.h>
#include "font.h"
#include "buffer.h"

struct segment {
    int size, color;
};

struct fracture_list {
    struct segment items[1024];
    int count;
    int current_color;
    int current_size;
};

void fracture_enlist(struct fracture_list *list)
{
    if (list->count < 1024) { 
        list->items[list->count++] = (struct segment){list->current_size, list->current_color};
    }
}

void fracture_feed(struct fracture_list *list, int size, int color)
{
    if (list->current_color != color) {
        fracture_enlist(list);
        list->current_color = color;
        list->current_size = size;
    } else {
        list->current_size += size;
    }
}

bool found_number(const char *line, int *size_out)
{
    const char *start = line;
    if (!isdigit(*line))
        return false;
    while (isdigit(*line))
        line++;

    *size_out = line-start;
    return true;
}

bool fracture_string(struct fracture_list *list, const char *line, int *size_out)
{
    const char *start = line;
    if (*line != '"')
        return false;

    line++;
    fracture_feed(list, 1, 2);
    while (*line && *line != '"') {
        if (strncmp(line, "\\\"", 2) == 0) {
            fracture_feed(list, 2, 3);
            line++;
        } else {
            fracture_feed(list, 1, 2);
        }
        line++;
    }
    fracture_feed(list, 1, 2);

    *size_out = ++line-start;
    return true;
}

struct fracture_list fracture_numbers(const char *line)
{
    struct fracture_list list = {0};
    int size;

    while (*line) {
        if (fracture_string(&list, line, &size))
            ;
        else if (found_number(line, &size))
            fracture_feed(&list, size, 1);
        else
            fracture_feed(&list, (size = 1), 0);
        line += size;
    }

    /* push the final element */
    fracture_enlist(&list);

    return list;
}

/* draws fractured line
 *------------------------------------
 * returns: position plus size of last character */
void render_line_fractures(char *line, SDL_Renderer *renderer, struct font *font, SDL_Point *at, struct fracture_list *list)
{
    SDL_Color presets[4] = {
        {250, 220, 190, 255},
        {201, 134,   0, 255},
        { 95, 135,  35, 255},
        {224,  92, 123, 255},
    };

    SDL_Point size;
    int initial_x = at->x;

    for (int i = 0; i < list->count; ++i) {
        struct segment segment = list->items[i];
        SDL_Color color = presets[segment.color];
        int previous_char = line[segment.size];
        line[segment.size] = 0;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        size = font_write_text(font, line, *at, renderer);
        line[segment.size] = previous_char;
        at->x += size.x;

        line += segment.size;
    }

    at->x = initial_x;
    at->y += size.y;
}

void dump_fracture_segment_list(struct fracture_list *list)
{
    printf("fracture_list {\n");
    for (int i = 0; i < list->count; ++i) {
        printf("    size %d | color %d\n", list->items[i].size, list->items[i].color);
    }
    printf("}\n");
}

/* draws buffer with syntax highlighting
 *--------------------------------------         
 * returns: position plus size of last character */
SDL_Point draw_buffer(SDL_Renderer *screen, struct font *font, struct buffer *buffer, SDL_Rect viewport, SDL_Color color)
{
    /* start rendering text from the viewport position */
    SDL_Point at = {viewport.x, viewport.y};

    SDL_SetRenderDrawColor(screen, color.r, color.g, color.b, color.a);

    for (int line = 0; line < buffer->line_count; ++line) {
        char *text = buffer->lines[line].data;
        struct fracture_list fractured_line = fracture_numbers(text);
        printf("Line %d\n", line+1);
        dump_fracture_segment_list(&fractured_line);
        render_line_fractures(text, screen, font, &at, &fractured_line);
    }

    return at;
}
