#include <SDL2/SDL_rect.h>

struct rectpack {
    struct rectpack_rect {
        int padding;
        SDL_Rect rect;
    } *rects;
    int rect_count;
};

struct rectpack rectpack_init();
void rectpack_add(struct rectpack *pack, SDL_Point size, int padding);
// Returns amount of rectangles that fit
int rectpack_fit(struct rectpack *pack, SDL_Point parent_size);
void rectpack_deinit(struct rectpack *pack);