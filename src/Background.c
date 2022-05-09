#include "Redactor.h"

void Background_Draw(Redactor *rs, Background *bg)
{
    int xcount = 0, ycount = 0;
    int texture_w, texture_h;
    SDL_QueryTexture(bg->texture, NULL, NULL, &texture_w, &texture_h);

    // NOTE: Fix zero divide
    if (texture_w == 0 || texture_h == 0)  {
        return;
    }
    
    if (bg->bgm_flags | Bgm_Tiled) {
        int screen_w, screen_h;
        SDL_GetWindowSize(rs->render_sdl_window, &screen_w, &screen_h);
        xcount = screen_w / texture_w + 2;
        ycount = screen_h / texture_h + 2;
    } 
    
    for (int x = 0; x < xcount; ++x) {
        for (int y = 0; y < ycount; ++y) {
            SDL_RenderCopy(rs->render_sdl_renderer, bg->texture, NULL, &(SDL_Rect){x*texture_w, y*texture_w, texture_w, texture_h});
        }
    }
}
