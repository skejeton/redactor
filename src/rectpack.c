#include "rectpack.h"

struct rectpack rectpack_init()
{
    return (struct rectpack) { 0 }; 
}

void rectpack_add(struct rectpack *pack, SDL_Point size, int padding)
{
    if (pack->rect_count % 32 == 0)
        pack->rects = realloc(pack->rects, (pack->rect_count+32)*sizeof(*pack->rects));
    pack->rects[pack->rect_count++] = (struct rectpack_rect) {padding, {0, 0, size.x, size.y}};
}

static SDL_Rect get_padded_rect(SDL_Rect rect, int padding)
{
    return (SDL_Rect) {rect.x, rect.y, rect.w + padding*2, rect.h + padding*2};
}

int rectpack_fit(struct rectpack *pack, SDL_Point parent_size)
{
    SDL_Point xy = {0, 0};
    int max_y = 0;

    for (int i = 0; i < pack->rect_count; ++i) {
        struct rectpack_rect rect = pack->rects[i];
        SDL_Rect to_insert = get_padded_rect(rect.rect, rect.padding);

        while (to_insert.w+xy.x > parent_size.x) {
            xy.y += max_y;
            xy.x = 0;
            max_y = to_insert.h;
            if (to_insert.h+xy.y > parent_size.y) {
                return i;
            }
        }

        pack->rects[i].rect.x = xy.x+rect.padding;
        pack->rects[i].rect.y = xy.y+rect.padding;
        xy.x += to_insert.w;
        max_y = to_insert.h > max_y ? to_insert.h : max_y;
    }
    return pack->rect_count;
}

void rectpack_deinit(struct rectpack *pack)
{
    free(pack->rects);
}
