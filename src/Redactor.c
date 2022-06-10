// Put includes here
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Utf8.h"
#include "Redactor.h"
#include "BufferDraw.h"
#include "HighlightSets.h"

// -- util

char *Redactor_GetTempResPath(Redactor *rs, const char *resname)
{
    free(rs->temp_respath);
    return (rs->temp_respath = Util_ConcatPaths(rs->program_dataPath, resname));
}

void Redactor_PrintMeta(Redactor *rs)
{
    printf("|-- redactor meta -------------\n");
    printf("|   cfg_program_dataDir  | %s\n", rs->cfg_program_dataDir);
    printf("|   cfg_font_respath     | %s\n", rs->cfg_font_respath);
    printf("|   cfg_font_size        | %d\n", rs->cfg_font_size);
    printf("|   program_location     | %s\n", rs->program_location);
    printf("|   program_dataPath     | %s\n", rs->program_dataPath);
    printf("|   file_name            | %s\n", rs->file_name);
    printf("|   file_is_new          | %d\n", rs->file_is_new);
    printf("|-- file_data -----------------\n");
    for (int i = 0; i < rs->file_buffer.lines_len; ++i) {
        printf("%s\n", rs->file_buffer.lines[i].text);
    }
    printf("|-- end redactor meta ---------\n");
}

void Redactor_PackCharTab(Redactor *rs, int page)
{

    int sfw = 300, sfh = 300, x = 0, y = 0, maxh = 0, padding = 0;
    SDL_Surface *dsf = SDL_CreateRGBSurfaceWithFormat(0, sfw, sfh, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_SetSurfaceBlendMode(dsf, SDL_BLENDMODE_BLEND);
    SDL_FillRect(dsf, NULL, SDL_MapRGBA(dsf->format, 0, 0, 0, 0));

    rs->render_font_chunks[page] = calloc(sizeof(GlyphChunk), 1);

    for (int i = 0; i < 256; ++i) {
        SDL_Surface *chsf = TTF_RenderGlyph32_Blended(rs->render_sdl_font_handle, i+page*256, (SDL_Color){255, 255, 255, 255});
        SDL_SetSurfaceBlendMode(chsf, SDL_BLENDMODE_NONE);
        
        if (chsf) {
            // NOTE: Destination only
            int chsfw = chsf->w+padding*2;
            int chsfh = chsf->h+padding*2;

            // NOTE: Check if can't fit in row anymore
            while (chsfw > (sfw-x)) {
                x = 0;
                y += maxh;
                maxh = chsfh;
            }

            // NOTE: Check if can't fit in column anymore (means we cant fit all chars -- total fail)
            if (chsfh > (sfh-y)) {
                // TODO: Instead resize surface and try again
                DieErr("Too tight to pack all ascii chars");
            }

            if (chsfh > maxh) {
                maxh = chsfh;
            }


            SDL_Rect dest = (SDL_Rect){x+padding, y+padding, chsf->w, chsf->h};
            rs->render_font_chunks[page]->glyphs[i] = dest;
            SDL_BlitSurface(chsf, &(SDL_Rect){0, 0, chsf->w, chsf->h}, dsf, &dest);
            x += chsfw;

            SDL_FreeSurface(chsf);
        }
    }

    rs->render_font_chunks[page]->atlas = SDL_CreateTextureFromSurface(rs->render_sdl_renderer, dsf);
    SDL_SetTextureBlendMode(rs->render_font_chunks[page]->atlas, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(dsf);
}

// -- init/deinit

void Redactor_Init(Redactor *rs)
{
    // -- Init default cfg 
    rs->cfg_program_dataDir = "data";
    rs->cfg_font_respath = "monospace.ttf";
    rs->cfg_font_size = 14;

    // -- Init paths
    rs->program_location = Util_GetProgramPath();
    rs->program_dataPath = Util_ConcatPaths(rs->program_location, rs->cfg_program_dataDir);

    // -- Init sdl
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        DieErr("Fatal: Can not init SDL: %s\n", SDL_GetError());
    }

    if (TTF_Init() < 0) {
        DieErr("Fatal: Can not init SDL_ttf: %s\n", TTF_GetError());
    }

    rs->render_sdl_window = SDL_CreateWindow("redactor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920/2, 1080/2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!rs->render_sdl_window) {
        DieErr("Fatal: Can not init window: %s\n", SDL_GetError());
    }

    rs->render_sdl_renderer = SDL_CreateRenderer(rs->render_sdl_window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!rs->render_sdl_renderer) {
        DieErr("Fatal: Can not init renderer: %s\n", SDL_GetError());
    }

    SDL_SetRenderDrawBlendMode(rs->render_sdl_renderer, SDL_BLENDMODE_BLEND);

    // -- Init font
    rs->render_sdl_font_handle = TTF_OpenFont(Redactor_GetTempResPath(rs, rs->cfg_font_respath), rs->cfg_font_size);

    if (!rs->render_sdl_font_handle) {
        DieErr("Fatal: Can not font: %s\n", TTF_GetError());
    }

    rs->render_font_height = TTF_FontHeight(rs->render_sdl_font_handle);

    // NOTE: ASCII table must be pre-mapped
    Redactor_PackCharTab(rs, 0);

    // -- Init values
    rs->program_running = true;
    rs->toy_textureViewer_scale = 1;
    rs->file_buffer = Buffer_Init();

    // TODO: Handle loading resources better
    SDL_Surface *bgSurface = SDL_LoadBMP(Redactor_GetTempResPath(rs, "debgtool.bmp"));
    rs->toy_textureViewer_bg.texture = SDL_CreateTextureFromSurface(rs->render_sdl_renderer, bgSurface);
    SDL_FreeSurface(bgSurface);
    rs->toy_textureViewer_bg.bgm_flags = 1;

    // -- Misc
}

void Redactor_UseArgs(Redactor *rs, int argc, char *argv[])
{
    if (argc != 2) {
        DieErr("Usage: %s file.txt\n", argv[0]);
    }

    rs->file_name = argv[1];
    rs->file_handle = fopen(rs->file_name, "r+");
    if (!rs->file_handle) {
        // TODO: This will create upon opening it,
        //       I should probably create it after write.
        rs->file_handle = fopen(rs->file_name, "w+");
        if (!rs->file_handle) {
            DieErr("Fatal: Error opening file %s: %s\n", rs->file_name, strerror(errno));
        } else {
            rs->file_is_new = true;
        }
    }

    char *file_str = Util_ReadFileStr(rs->file_handle);

    Buffer_InsertUTF8(&rs->file_buffer, rs->file_cursor, file_str);

    free(file_str);
}

void Redactor_End(Redactor *rs)
{
    // NOTE: Allocated in Highlighting
    BufferDraw_InvalidateSegments(&rs->render_drawSegments);
    // NOTE: Allocated in UseArgs
    Buffer_Deinit(&rs->file_buffer);
    // NOTE: Allocated in GetTempResPath
    free(rs->temp_respath);
    // NOTE: Allocated in SetupPaths
    free(rs->program_location);
    free(rs->program_dataPath);

    TTF_CloseFont(rs->render_sdl_font_handle);
    SDL_DestroyTexture(rs->toy_textureViewer_bg.texture);
    for (int i = 0; i < Redactor_GlyphmapChunkMax; ++i) {
        if (rs->render_font_chunks[i]) {
            SDL_DestroyTexture(rs->render_font_chunks[i]->atlas);
            free(rs->render_font_chunks[i]);
        }
    }
    SDL_DestroyRenderer(rs->render_sdl_renderer);
    SDL_DestroyWindow(rs->render_sdl_window); 
    fclose(rs->file_handle);
    TTF_Quit();
    SDL_Quit();
}

// -- draw

SDL_Point Redactor_DrawText(Redactor *rs, SDL_Color color, const char *text, int initx, int x, int y, int col)
{
    int c;
    if (y < -rs->render_font_height || y > rs->render_window_size.y) {
        return (SDL_Point){x, y};
    }
    
    while ((c = Utf8_NextVeryBad(&text))) {
        // NOTE: Prevent out of bounds
        if (c < 0 || c >= Redactor_GlyphmapGlyphMax) {
            continue;
        }

        if (c == '\n') {
            y += rs->render_font_chunks[0]->glyphs[' '].h;
            x = initx;
            continue;
        } else if (c == '\t') {
            x += rs->render_font_chunks[0]->glyphs[' '].w * (8 - (col % 8));
            col += (8 - (col % 8));
            continue;
        }

        if (rs->render_font_chunks[c / 256] == NULL) {
            Redactor_PackCharTab(rs, c / 256);
        }

        GlyphChunk *chunk = rs->render_font_chunks[c / 256];
        SDL_SetTextureColorMod(chunk->atlas, color.r, color.g, color.b);

        SDL_Rect src = chunk->glyphs[c%256];
        SDL_RenderCopy(rs->render_sdl_renderer, chunk->atlas, &src, &(SDL_Rect){x, y, src.w, src.h});
        x += src.w;
        col++;
    }

    return (SDL_Point){x, y};
}

SDL_Rect Redactor_GetCursorRect(Redactor *rs)
{
    Cursor cursor = rs->file_cursor;
    
    int x = 0, y = 0;
    int col = 0;
    int h = rs->render_font_height;
    y = cursor.line * h;

    const char *line = rs->file_buffer.lines[cursor.line].text;

    for (int c, i = 0; (c = Utf8_NextVeryBad(&line)); ++i) {
        if (i == cursor.column) {
            break;
        }

        // NOTE: Prevent out of bounds
        if (c < 0 || c >= Redactor_GlyphmapGlyphMax) {
            continue;
        }

        if (c == '\t') {
            x += rs->render_font_chunks[0]->glyphs[' '].w * (8 - (col % 8));
            col += (8 - (col % 8));
            continue;
        }

        x += rs->render_font_chunks[c / 256]->glyphs[c % 256].w;
        col += 1;
    }
    
    return (SDL_Rect){rs->render_scroll.x+x, rs->render_scroll.y+y, 2, h};
}

void Redactor_MoveCursorToVisibleArea(Redactor *rs)
{
    SDL_Rect cursor_rect = Redactor_GetCursorRect(rs);

    if (cursor_rect.y < 0) {
        rs->render_scroll.y -= cursor_rect.y;
    }
    if ((cursor_rect.y + cursor_rect.h) > rs->render_window_size.y) {
        rs->render_scroll.y -= (cursor_rect.y + cursor_rect.h) - rs->render_window_size.y;
    }
}

void Redactor_DrawCursor(Redactor *rs) 
{
    SDL_Rect cursor_rect = Redactor_GetCursorRect(rs);

    SDL_SetRenderDrawColor(rs->render_sdl_renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(rs->render_sdl_renderer, &cursor_rect);
}

// NOTE: For debugging
void Redactor_DrawTextureViewer(Redactor *rs, SDL_Texture *texture)
{
    float scale = rs->toy_textureViewer_scale;
    int texture_w, texture_h, screen_w, screen_h, tex_pos_x, tex_pos_y;
    SDL_QueryTexture(texture, NULL, NULL, &texture_w, &texture_h);
    SDL_GetWindowSize(rs->render_sdl_window, &screen_w, &screen_h);
    
    texture_w *= scale;
    texture_h *= scale;

    tex_pos_x = (screen_w - texture_w) / 2;
    tex_pos_y = (screen_h - texture_h) / 2;

    SDL_SetRenderDrawColor(rs->render_sdl_renderer, 0, 0, 80, 255);
    Background_Draw(rs, &rs->toy_textureViewer_bg);
    char title[1024];
    snprintf(title, 1024, "Texture viewer | w %d | h %d | s %g", texture_w, texture_h, scale);

    Redactor_DrawText(rs, Redactor_Color_Fore, title, tex_pos_x, tex_pos_x, tex_pos_y-20, 0);
    SDL_SetRenderDrawColor(rs->render_sdl_renderer, 70, 50, 128, 128);
    SDL_RenderDrawRect(rs->render_sdl_renderer, &(SDL_Rect){tex_pos_x-2, tex_pos_y-2, texture_w+4, texture_h+4});
}

// -- control
void Redactor_SetCursorAtScreenPos(Redactor *rs, int x, int y)
{
    int line = (y - rs->render_scroll.y) / rs->render_font_height;

    if (line < 0) {
        line = 0;
    } else if (line > rs->file_buffer.lines_len-1) {
        line = rs->file_buffer.lines_len-1;
    }
    rs->file_cursor.line = line;

    Line linedata = rs->file_buffer.lines[line];
    int column = 0;
    int col = 0;
    int mindist = x;
    int cx = 0;

    for (int c; (c = Utf8_NextVeryBad((const char**)&linedata.text)); ++column) {
        // NOTE: Prevent out of bounds
        if (c < 0 || c >= Redactor_GlyphmapGlyphMax) {
            continue;
        }

        if (c == '\t') {
            cx += rs->render_font_chunks[0]->glyphs[' '].w * (8 - (col % 8));
            col += (8 - (col % 8));
        } else {
            cx += rs->render_font_chunks[c / 256]->glyphs[c % 256].w;
            col++;
        }

        int dist = abs(cx - x);

        if (dist > mindist) {
            break;
        } else {
            mindist = dist;
        }
    }

    rs->file_cursor.column = column;
}

void Redactor_ScrollScreen(Redactor *rs, int byX, int byY)
{
    rs->render_scroll.y += byY;
    int hlimit = -((int)rs->file_buffer.lines_len*rs->render_font_height)+rs->render_window_size.y;
    
    // TODO: Figure out why scroll is so weird, it goes into negatives?

    if (rs->render_scroll.y < hlimit)
        rs->render_scroll.y = hlimit;

    if (rs->render_scroll.y > 0)
        rs->render_scroll.y = 0;
}

void Redactor_Draw(Redactor *rs)
{
    SDL_SetRenderDrawColor(rs->render_sdl_renderer, 17, 41, 31, 255);
    SDL_RenderClear(rs->render_sdl_renderer);
    BufferDraw_DrawBuffer(rs, &rs->file_buffer, &rs->render_drawSegments);
    Redactor_DrawCursor(rs);
    SDL_RenderPresent(rs->render_sdl_renderer);
}

void In_InvalidateBuffer(Redactor *rs) {
    Highlight_HighlightBuffer(&rs->file_buffer, &HighlightSets_C, &rs->render_drawSegments);
}

void Redactor_HandleEvents(Redactor *rs)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            rs->program_running = false;
            break;
        case SDL_MOUSEWHEEL:
            rs->toy_textureViewer_scale += event.wheel.y/10.0;
            Redactor_ScrollScreen(rs, 0, event.wheel.y * rs->render_font_height);
            break;
        case SDL_TEXTINPUT:
            rs->file_cursor = Buffer_InsertUTF8(&rs->file_buffer, rs->file_cursor, event.text.text);
            In_InvalidateBuffer(rs);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                Redactor_SetCursorAtScreenPos(rs, event.button.x, event.button.y);
                Redactor_MoveCursorToVisibleArea(rs);
            } 
            break;
        case SDL_KEYUP: {
            switch (event.key.keysym.scancode) {
                // -- control keys
                case SDL_SCANCODE_LCTRL:
                case SDL_SCANCODE_RCTRL:
                    rs->input.ks_ctrl = false;
                    break;
                default:;
            }
        } break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode) {
            // -- control keys
            case SDL_SCANCODE_LCTRL:
            case SDL_SCANCODE_RCTRL:
                rs->input.ks_ctrl = true;
                break;
            // -- control keystrokes
            case SDL_SCANCODE_V: 
                {
                    if (rs->input.ks_ctrl) {
                        char *clipboardText = SDL_GetClipboardText();
                        rs->file_cursor = Buffer_InsertUTF8(&rs->file_buffer, rs->file_cursor, clipboardText);
                        free(clipboardText);
                        In_InvalidateBuffer(rs);
                    }
                }
                break;
            // -- miscel
            case SDL_SCANCODE_TAB:
                rs->file_cursor = Buffer_InsertUTF8(&rs->file_buffer, rs->file_cursor, "\t");
                Redactor_MoveCursorToVisibleArea(rs);
                In_InvalidateBuffer(rs);
                break;
            case SDL_SCANCODE_RETURN:
                rs->file_cursor = Buffer_InsertUTF8(&rs->file_buffer, rs->file_cursor, "\n");
                Redactor_MoveCursorToVisibleArea(rs);
                In_InvalidateBuffer(rs);
                break;
            case SDL_SCANCODE_BACKSPACE:
                rs->file_cursor = Buffer_RemoveCharacterUnder(&rs->file_buffer, rs->file_cursor);
                Redactor_MoveCursorToVisibleArea(rs);
                In_InvalidateBuffer(rs);
                break;
            case SDL_SCANCODE_UP:
                rs->file_cursor = Buffer_MoveCursor(&rs->file_buffer, rs->file_cursor, -1, 0);
                Redactor_MoveCursorToVisibleArea(rs);
                break;
            case SDL_SCANCODE_DOWN:
                rs->file_cursor = Buffer_MoveCursor(&rs->file_buffer, rs->file_cursor, 1, 0);
                Redactor_MoveCursorToVisibleArea(rs);
                break;
            case SDL_SCANCODE_LEFT:
                rs->file_cursor = Buffer_MoveCursor(&rs->file_buffer, rs->file_cursor, 0, -1);
                Redactor_MoveCursorToVisibleArea(rs);
                break;
            case SDL_SCANCODE_RIGHT:
                rs->file_cursor = Buffer_MoveCursor(&rs->file_buffer, rs->file_cursor, 0, 1);
                Redactor_MoveCursorToVisibleArea(rs);
                break;
            default:;
            }
            break;
        default:;
        }
    }
}

void Redactor_Cycle(Redactor *rs)
{
    Redactor_HandleEvents(rs);
    
    SDL_GetWindowSize(rs->render_sdl_window, &rs->render_window_size.x, &rs->render_window_size.y);
    Redactor_Draw(rs);
}

int Redactor_Main(int argc, char *argv[])
{
    Redactor rs = {0};
    Redactor_Init(&rs);
    Redactor_UseArgs(&rs, argc, argv);
    In_InvalidateBuffer(&rs);

    while (rs.program_running) {
        Redactor_Cycle(&rs);
    }

    Redactor_End(&rs);
    return 0;
}
