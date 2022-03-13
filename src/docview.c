#include "docview.h"
#include "buftext.h"

void dv_draw(struct docview *view, SDL_Renderer *renderer)
{
    struct buftext_pass text_pass = {
        .buffer = &view->document.buffer,
        .font = view->font,
        .renderer = renderer,
        .color = {250, 220, 190, 255},
        .viewport = view->viewport,
        .scroll = view->scroll
    };

    draw_buffer_text(&text_pass);
}

void dv_scroll(struct docview *view, float dx, float dy)
{
    view->scroll.x += dx;
    view->scroll.y += dy;
}

void dv_tap(struct docview *view, bool shift, SDL_Point xy)
{

}