
// Put includes here
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>


#include "Unicode.h"
#include "Redactor.h"




// -- buffer

Cursor Redactor_Buffer_InsertUTF8(Redactor *rs, Cursor cursor, const char *text)
{
        Line *line = &rs->file_buffer.lines[cursor.line];
        size_t new_size = strlen(text) + line->text_size;
        char *new_line_start = malloc(sizeof(char) * (new_size + 1));
        char *new_line = new_line_start;
        int cursor_pos_byte = 0;
        int unilen = Uni_Utf8_Strlen(text);
        const char *line_iter = line->text;

        // --find cursor
        for (int i = 0; i != cursor.column && Uni_Utf8_NextVeryBad(&line_iter); ++i)
                ;
        cursor_pos_byte = line_iter - line->text;

        // --before
        for (int i = 0; i < cursor_pos_byte; ++i)
                *new_line++ = line->text[i];
        // --mid
        for (int i = 0; text[i]; ++i)
                *new_line++ = text[i];
        // --after
        for (int i = cursor_pos_byte; line->text[i]; ++i)
                *new_line++ = line->text[i];

        *new_line = 0;

        free(line->text);
        line->text = new_line_start;
        line->text_size = new_size;
        line->text_len += unilen;

        cursor.column += unilen;
        return cursor;
}

Cursor Redactor_Buffer_RemoveCharacter(Redactor *rs, Cursor under)
{
        if (under.column == 0) {
                if (under.line > 0) {
                        under.line--;
                        // TODO: Inline this with a specialized version
                        Line *line = &rs->file_buffer.lines[under.line];
                        under.column = line->text_len;
                        Redactor_Buffer_InsertUTF8(rs, under, rs->file_buffer.lines[under.line+1].text);

                        free(rs->file_buffer.lines[under.line+1].text);
                        memmove(rs->file_buffer.lines+under.line+1, rs->file_buffer.lines+under.line+2, sizeof(*rs->file_buffer.lines) * (rs->file_buffer.lines_len - (under.line+2)));
                        rs->file_buffer.lines_len -= 1;
                }
        } else {
                Line *line = &rs->file_buffer.lines[under.line];

                const char *line_iter = line->text;
                // NOTE: character before cursor
                const char *p_line_iter = line->text;
                
                // --find cursor
                for (int i = 0; i != under.column && Uni_Utf8_NextVeryBad((p_line_iter = line_iter, &line_iter)); ++i)
                        ;

                memmove(line->text + (p_line_iter - line->text), line->text + (line_iter - line->text), sizeof(char) * (line->text_size - (line_iter - line->text) + 1));

                line->text_len -= 1;

                under.column -= 1;
        }
        return under;
}

Cursor Redactor_Buffer_MoveCursorColumns(Redactor *rs, Cursor cursor, int col)
{
        int column = (int)(cursor.column) + col;

        if (column < 0) {
                while (column < 0 && cursor.line > 0) {
                        cursor.line--;
                        column += rs->file_buffer.lines[cursor.line].text_len+1;
                }
        } else {
                while (column > rs->file_buffer.lines[cursor.line].text_len && cursor.line < rs->file_buffer.lines_len-1) {
                        column -= rs->file_buffer.lines[cursor.line].text_len+1;
                        cursor.line++;
                }
        }
        if (column < 0) {
                column = 0; 
        } else if (column > rs->file_buffer.lines[cursor.line].text_len) {
                column = rs->file_buffer.lines[cursor.line].text_len;
        }

        cursor.column = column;


        return cursor;
}


Cursor Redactor_Buffer_MoveCursor(Redactor *rs, Cursor cursor, int lines, int cols)
{
        cursor.line += lines;

        if (cursor.line < 0) {
                cursor.line = 0;
        } else if (cursor.line >= rs->file_buffer.lines_len) {
                cursor.line = rs->file_buffer.lines_len-1;
        }
                
        if (cursor.column < 0) {
                cursor.column = 0;
        } else if (cursor.column > rs->file_buffer.lines[cursor.line].text_len) {
                cursor.column = rs->file_buffer.lines[cursor.line].text_len;
        }

        return Redactor_Buffer_MoveCursorColumns(rs, cursor, cols);
}


void Redactor_Buffer_AddLine(Redactor *rs, const char *line)
{
        if (rs->file_buffer.lines_len % 1024 == 0) {
                rs->file_buffer.lines = realloc(rs->file_buffer.lines, sizeof(*rs->file_buffer.lines) * (rs->file_buffer.lines_len + 1024));
        }
        
        size_t text_size = strlen(line);
        char *text = strcpy(malloc(text_size+1), line);

        rs->file_buffer.lines[rs->file_buffer.lines_len  ].text_len  = Uni_Utf8_Strlen(text);
        rs->file_buffer.lines[rs->file_buffer.lines_len  ].text_size = text_size;
        rs->file_buffer.lines[rs->file_buffer.lines_len++].text      = text;
}

void Redactor_Buffer_AddEmptyLineAt(Redactor *rs, Cursor under)
{
        if (rs->file_buffer.lines_len % 1024 == 0) {
                rs->file_buffer.lines = realloc(rs->file_buffer.lines, sizeof(*rs->file_buffer.lines) * (rs->file_buffer.lines_len + 1024));
        }
        
        memmove(rs->file_buffer.lines+under.line+1, rs->file_buffer.lines+under.line, sizeof(*rs->file_buffer.lines) * (rs->file_buffer.lines_len-under.line));
        char *empty = malloc(1);
        empty[0] = 0;

        rs->file_buffer.lines[under.line].text_len = 0;
        rs->file_buffer.lines[under.line].text_size = 0;
        rs->file_buffer.lines[under.line].text = empty;
        rs->file_buffer.lines_len++;
}

Cursor Redactor_Buffer_SplitLineAt(Redactor *rs, Cursor under)
{
        Line *line = &rs->file_buffer.lines[under.line];
        line->text_len = under.column;

        char *line_text = line->text;
        const char *line_iter = line->text;
        // NOTE: character before cursor
        
        // --find cursor
        for (int i = 0; i != under.column && Uni_Utf8_NextVeryBad(&line_iter); ++i)
                ;
        line->text_size = line_iter-line->text;

        under.line += 1;
        under.column = 0;
        Redactor_Buffer_AddEmptyLineAt(rs, under);
        Redactor_Buffer_InsertUTF8(rs, under, line_iter);
        line_text[line_iter-line_text] = 0;
        return under;
}

void Redactor_Buffer_Deinit(Redactor *rs)
{
        for (int i = 0; i < rs->file_buffer.lines_len; ++i) {
                free(rs->file_buffer.lines[i].text);
        }
        free(rs->file_buffer.lines);
}

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
        rs->cfg_font_size = 16;

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
        // NOTE: ASCII table must be pre-mapped
        Redactor_PackCharTab(rs, 0);

        // -- Init values
        rs->program_running = true;
        rs->toy_textureViewer_scale = 1;

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

        char *file_str_init = Util_ReadFileStr(rs->file_handle);
        char *file_str = file_str_init;

        while (*file_str) {
                char *line = file_str;
                while (*file_str && *file_str != '\n') {
                        file_str++;
                }
                char nnul = *file_str;
                *file_str = 0;
                Redactor_Buffer_AddLine(rs, line);
                if (nnul) {
                        file_str++;
                }
        }

        // NOTE: If file is empty the first line might not be 
        if (*file_str_init == 0) {
                Redactor_Buffer_AddLine(rs, "");
        }


        free(file_str_init);
}

void Redactor_End(Redactor *rs)
{
        // NOTE: Allocated in UseArgs
        Redactor_Buffer_Deinit(rs);
        // NOTE: Allocated in GetTempResPath
        free(rs->temp_respath);
        // NOTE: Allocated in SetupPaths
        free(rs->program_location);
        free(rs->program_dataPath);

        TTF_CloseFont(rs->render_sdl_font_handle);
        SDL_DestroyTexture(rs->toy_textureViewer_bg.texture);
        for (int i = 0; i < Redactor_GlyphmapChunkMax; ++i) 
                if (rs->render_font_chunks[i]) {
                        SDL_DestroyTexture(rs->render_font_chunks[i]->atlas);
                        free(rs->render_font_chunks[i]);
                }
        SDL_DestroyRenderer(rs->render_sdl_renderer);
        SDL_DestroyWindow(rs->render_sdl_window); 
        fclose(rs->file_handle);
        TTF_Quit();
        SDL_Quit();
}

// -- draw

int Redactor_DrawText(Redactor *rs, int x, int y, const char *text)
{
        int c;
        int col = 0;

        while ((c = Uni_Utf8_NextVeryBad(&text))) {
                // NOTE: Prevent out of bounds
                if (c < 0 || c >= Redactor_GlyphmapGlyphMax) {
                        continue;
                }

                if (c == '\t') {
                        x += rs->render_font_chunks[0]->glyphs[' '].w * (8 - (col % 8));
                        col += (8 - (col % 8));
                        continue;
                }

                if (rs->render_font_chunks[c / 256] == NULL) {
                        Redactor_PackCharTab(rs, c / 256);
                }

                GlyphChunk *chunk = rs->render_font_chunks[c / 256];

                SDL_Rect src = chunk->glyphs[c%256];
                SDL_RenderCopy(rs->render_sdl_renderer, chunk->atlas, &src, &(SDL_Rect){x, y, src.w, src.h});
                x += src.w;
                col++;
        }

        // FIXME: This is a meh way to get line height
        return rs->render_font_chunks[0] ? rs->render_font_chunks[0]->glyphs[' '].h : 0;
}

SDL_Rect Redactor_GetCursorRect(Redactor *rs)
{
        Cursor cursor = rs->file_cursor;
        
        int x = 0, y = 0;
        int col = 0;
        // FIXME: This is a meh way to get line height
        int h = rs->render_font_chunks[0] ? rs->render_font_chunks[0]->glyphs[' '].h : 0;
        y = cursor.line * h;

        const char *line = rs->file_buffer.lines[cursor.line].text;

        for (int c, i = 0; (c = Uni_Utf8_NextVeryBad(&line)); ++i) {
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

void Redactor_DrawCursor(Redactor *rs) 
{
        SDL_Rect cursor_rect = Redactor_GetCursorRect(rs);

        if (cursor_rect.y < 0) {
                rs->render_scroll.y -= cursor_rect.y;
        }
        if ((cursor_rect.y + cursor_rect.h) > rs->render_window_size.y) {
                rs->render_scroll.y -= (cursor_rect.y + cursor_rect.h) - rs->render_window_size.y;
        }
        cursor_rect = Redactor_GetCursorRect(rs);

        SDL_SetRenderDrawColor(rs->render_sdl_renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(rs->render_sdl_renderer, &cursor_rect);
}

void Redactor_DrawDocument(Redactor *rs)
{
        int y = 0;
        for (int i = 0; i < rs->file_buffer.lines_len; ++i) {
                const char *line = rs->file_buffer.lines[i].text;
                y += Redactor_DrawText(rs, rs->render_scroll.x, rs->render_scroll.y+y, line);
        }
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

        Redactor_DrawText(rs, tex_pos_x, tex_pos_y-20, title);
        SDL_SetRenderDrawColor(rs->render_sdl_renderer, 70, 50, 128, 128);
        SDL_RenderDrawRect(rs->render_sdl_renderer, &(SDL_Rect){tex_pos_x-2, tex_pos_y-2, texture_w+4, texture_h+4});
        SDL_RenderCopy(rs->render_sdl_renderer, texture, NULL, &(SDL_Rect){tex_pos_x, tex_pos_y, texture_w, texture_h});
}

// -- control

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
                        break;
                case SDL_TEXTINPUT:
                        rs->file_cursor = Redactor_Buffer_InsertUTF8(rs, rs->file_cursor, event.text.text);
                        break;
                case SDL_KEYDOWN:
                        switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_TAB:
                                rs->file_cursor = Redactor_Buffer_InsertUTF8(rs, rs->file_cursor, "\t");
                                break;
                        case SDL_SCANCODE_RETURN:
                                rs->file_cursor = Redactor_Buffer_SplitLineAt(rs, rs->file_cursor);
                                break;
                        case SDL_SCANCODE_BACKSPACE:
                                rs->file_cursor = Redactor_Buffer_RemoveCharacter(rs, rs->file_cursor);
                                break;
                        case SDL_SCANCODE_UP:
                                rs->file_cursor = Redactor_Buffer_MoveCursor(rs, rs->file_cursor, -1, 0);
                                break;
                        case SDL_SCANCODE_DOWN:
                                rs->file_cursor = Redactor_Buffer_MoveCursor(rs, rs->file_cursor, 1, 0);
                                break;
                        case SDL_SCANCODE_LEFT:
                                rs->file_cursor = Redactor_Buffer_MoveCursor(rs, rs->file_cursor, 0, -1);
                                break;
                        case SDL_SCANCODE_RIGHT:
                                rs->file_cursor = Redactor_Buffer_MoveCursor(rs, rs->file_cursor, 0, 1);
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
        SDL_SetRenderDrawColor(rs->render_sdl_renderer, 255, 255, 255, 255);
        SDL_RenderClear(rs->render_sdl_renderer);
        Background_Draw(rs, &rs->toy_textureViewer_bg);
        Redactor_DrawDocument(rs);
        Redactor_DrawCursor(rs);
        /*
        if (rs->render_font_chunks[117]) {
                Redactor_DrawTextureViewer(rs, rs->render_font_chunks[117]->atlas);
        }
        */
        
        SDL_RenderPresent(rs->render_sdl_renderer);
}

int Redactor_Main(int argc, char *argv[])
{
        Redactor rs = {0};
        Redactor_Init(&rs);
        DieErr("Damn son\n");

        Redactor_UseArgs(&rs, argc, argv);
        Redactor_PrintMeta(&rs);
        while (rs.program_running) {
                Redactor_Cycle(&rs);
        }
        Redactor_End(&rs);
        return 0;
}
