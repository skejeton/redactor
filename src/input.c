#include "input.h"
#include "font.h"
#include "dbg.h"
#include "util.h"

static void change_document_font_size(struct docview *view, int delta, SDL_Renderer *renderer)
{
    int size = font_get_size(view->font) + delta;
    if (size < 5)
        size = 5;
    if (size > 40)
        size = 40;
    font_resize(view->font, size, renderer);
}

static bool is_selecting(struct docedit *editor)
{
    return !buffer_range_empty(editor->cursor.selection);
}

static int strchrdx(const char *s, int c)
{
    char *f = strchr(s, c);
    return f ? f - s : -1;
}

static int match_open_close_pairs(struct buffer *buffer, struct buffer_range range, const char *open, const char *closing)
{   
    if (strlen(open) != strlen(closing)) {
        fprintf(stderr, "Internal error: Open close pair length mismatch\n");
        return 0;
    }
    
    char stack[1024];
    int stack_size = 0;
    
    // TODO: Consider using a slower but memory efficent version, or a chunked one
    char *b = buffer_get_range(buffer, range);
    for (int i = 0; b[i]; i++) {
        if (strchr(open, b[i]) && stack_size < 1024) {
            stack[stack_size++] = b[i];
        } else if (strchr(closing, b[i]) && stack_size > 0) {
            char open_char = stack[stack_size-1];
            if (strchrdx(closing, b[i]) == strchrdx(open, open_char)) {
                --stack_size;
            } else {
                // Backtrack until matching is found, or empty
                while (strchrdx(open, stack[stack_size-1]) != strchrdx(closing, b[i]) && --stack_size > 0)
                    ;
            }
        }
    }
    free(b);
    return stack_size;
}

static struct buffer_marker get_cursor_position(struct docedit *editor)
{
    return editor->cursor.selection.to;
}

static int calculate_tabulation_spaces(struct docedit *editor)
{
    struct buffer_marker at = get_cursor_position(editor);
    // Count from the start up until a non space character
    at.column = 0;
    int c;
    while ((c = buffer_get_char(&editor->buffer, at)) == ' ') {
        at.column++;
    }

    return at.column;
}

static bool is_cursor_below_content(struct docedit *editor)
{
    if (!is_selecting(editor)) {
        struct buffer_marker at = get_cursor_position(editor);
        int spaces = calculate_tabulation_spaces(editor);

        if (at.column > spaces || at.column == 0)
            return false;
    }
    return true;
}

static bool erase_tabulation(struct docedit *editor)
{
    if (!is_selecting(editor)) {

        struct buffer_marker at = get_cursor_position(editor);
        int spaces = calculate_tabulation_spaces(editor);
        // Ignore if there was a non space character previously
        if (at.column > spaces || at.column == 0)
            return 0;


        if (spaces > 4)
            spaces = 4;

        // Adjust column to nearest 4th
        at.column -= (at.column - spaces + 3) % 4 + 1;

        if (spaces > 0) {
            // Set the selection and erase the text
            editor->cursor.selection.from = at;
            de_erase(editor);
        }
        return spaces;
    }
    return 0;
}

void input_process_event(struct input_state *state, struct input_pass pass)
{
    struct docedit *editor = &pass.view->document;
    struct docview *view = pass.view;
    SDL_Event *event = pass.event;

    int sw, sh;
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_GetWindowSize(pass.window, &sw, &sh);

    switch (event->type) {
    case SDL_TEXTINPUT: {
        if (!state->ctrl) {
            if (is_cursor_below_content(editor) && strchr(")}]", event->text.text[0])) {
                erase_tabulation(editor);
            }
            de_insert(editor, event->text.text);
        }
    } break;
    case SDL_MOUSEWHEEL:
        if (state->ctrl) {
            change_document_font_size(view, event->wheel.y, pass.renderer);
        } else {   
            dv_scroll(view, 0, event->wheel.y*30.0);
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        state->leftmousedown = event->button.button == SDL_BUTTON_LEFT && event->type == SDL_MOUSEBUTTONDOWN;
        if (state->leftmousedown) {
            dv_tap(view, state->shift, (SDL_Point){event->button.x, event->button.y});
        }    
        break;
    case SDL_WINDOWEVENT:
        switch (event->window.event) {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            state->focused = 1;
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            state->focused = 0;
            break;
        }
        break;
    case SDL_KEYDOWN:
        switch (event->key.keysym.scancode) {
        case SDL_SCANCODE_S:
            if (state->ctrl) {
                editor->buffer.dirty = 0;
                // FIXME: make a proper buffer bound function
                char *file = buffer_get_range(&editor->buffer, (struct buffer_range) {{100000, 100000}});
                util_write_whole_file(pass.filename, file);
                free(file);
            }
            break;
        case SDL_SCANCODE_X:
            if (state->ctrl) {
                char *c = de_get_selection(editor);
                SDL_SetClipboardText(c);
                de_erase(editor);
                free(c);
            }
            break;
        case SDL_SCANCODE_C:
            if (state->ctrl) {
                char *c = de_get_selection(editor);
                SDL_SetClipboardText(c);
                free(c);
            }
            break;
        case SDL_SCANCODE_V:
            if (state->ctrl) {
                char *s = SDL_GetClipboardText();
                de_insert(editor, s);
                SDL_free(s);
            }
            break;
        case SDL_SCANCODE_MINUS:
            if (state->ctrl) {
                change_document_font_size(view, -1, pass.renderer);
            }
            break;
        case SDL_SCANCODE_EQUALS:
            if (state->ctrl) {
                change_document_font_size(view, 1, pass.renderer);
            }
            break;
        case SDL_SCANCODE_BACKSPACE:
            if (!erase_tabulation(editor))
                de_erase(editor);
            break;
        case SDL_SCANCODE_UP:
            de_move_cursor(editor, state->shift, 0, -1);
            break;
        case SDL_SCANCODE_DOWN:
            de_move_cursor(editor, state->shift, 0, 1);
            break;
        case SDL_SCANCODE_LEFT:
            de_move_cursor(editor, state->shift, -1, 0);
            break;
        case SDL_SCANCODE_RIGHT:
            de_move_cursor(editor, state->shift, 1, 0);
            break;
        case SDL_SCANCODE_RETURN: {
            int spaces = match_open_close_pairs(&editor->buffer, (struct buffer_range) {{0}, editor->cursor.selection.to}, "({[", ")}]") * 4;
            int persistspaces = calculate_tabulation_spaces(editor);
            if (persistspaces >= spaces) {
                if (persistspaces > get_cursor_position(editor).column)
                    persistspaces = get_cursor_position(editor).column;
                spaces += persistspaces-spaces;
            }
           de_insert(editor, "\n");
            while (spaces--)
                de_insert(editor, " ");
        } break;
        case SDL_SCANCODE_TAB:
            for (int i = 0, col = editor->cursor.selection.from.column; i < (4 - col % 4); i++)
                de_insert(editor, " ");
            break;
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:
            state->ctrl = 1;
            break;
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:
            state->shift = 1;
            break;
        default:;
        }
        break;
    case SDL_KEYUP:
        if (event->key.keysym.scancode == SDL_SCANCODE_LCTRL ||
            event->key.keysym.scancode == SDL_SCANCODE_RCTRL)
            state->ctrl = 0;
        if (event->key.keysym.scancode == SDL_SCANCODE_LSHIFT ||
            event->key.keysym.scancode == SDL_SCANCODE_RSHIFT)
            state->shift = 0;
        break;
    case SDL_QUIT:
        *pass.running = 0;
    }
}
