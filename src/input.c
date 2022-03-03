#include "input.h"
#include "font.h"
#include "util.h"

static void change_document_font_size(struct docview *view, int delta, SDL_Renderer *renderer)
{
    int size = font_size(view->font) + delta;
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


static struct buffer_marker get_cursor_position(struct docedit *editor)
{
    return editor->cursor.selection.to;
}

static struct buffer_marker get_beneath_cursor_position(struct docedit *editor)
{
    struct buffer_marker marker = editor->cursor.selection.to;
    marker.column -= 1; // This makes that the cursor points to the character it's beneath
    return marker;
}

static int calculate_tabulation_spaces(struct docedit *editor)
{
    struct buffer_marker at = get_beneath_cursor_position(editor);
    int spaces = 0, c;
    while ((c = buffer_get_char(&editor->buffer, at)) == ' ') {
        spaces++;
        at.column--;
    }

    return !c ? spaces : 0;
}

static bool erase_tabulation(struct docedit *editor)
{
    if (!is_selecting(editor)) {

        int spaces = calculate_tabulation_spaces(editor);
        if (spaces > 4)
            spaces = 4;

        struct buffer_marker at = get_cursor_position(editor);
        // Adjust column to nearest 4th
        at.column -= (at.column - spaces + 3) % 4 + 1;

        if (spaces > 0) {
            // Set the selection and erase the text
            editor->cursor.selection.from = at;
            docedit_erase(editor);
        }
        return spaces;
    }
    return 0;
}

void input_process_event(struct input_state *state, struct input_pass pass)
{
    struct docedit *editor = &pass.view->doc;
    struct docview *view = pass.view;
    SDL_Event *event = pass.event;

    int sw, sh;
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_GetWindowSize(pass.window, &sw, &sh);

    switch (event->type) {
    case SDL_TEXTINPUT: {
        if (!state->ctrl) {
            docedit_insert(editor, event->text.text);
        }
    } break;
    case SDL_MOUSEWHEEL:
        if (state->ctrl) {
            change_document_font_size(view, event->wheel.y, pass.renderer);
        } else {   
            view->scroll.y -= event->wheel.y*30;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        state->leftmousedown = event->button.button == SDL_BUTTON_LEFT && event->type == SDL_MOUSEBUTTONDOWN;
        if (state->leftmousedown) {
            docview_tap(false, (SDL_Point) {mouse_x, mouse_y}, view);
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
                char *c = docedit_get_selection(editor);
                SDL_SetClipboardText(c);
                docedit_erase(editor);
                free(c);
            }
            break;
        case SDL_SCANCODE_C:
            if (state->ctrl) {
                char *c = docedit_get_selection(editor);
                SDL_SetClipboardText(c);
                free(c);
            }
            break;
        case SDL_SCANCODE_V:
            if (state->ctrl) {
                char *s = SDL_GetClipboardText();
                docedit_insert(editor, s);
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
                docedit_erase(editor);
            break;
        case SDL_SCANCODE_UP:
            docedit_move_cursor(editor, state->shift, 0, -1);
            break;
        case SDL_SCANCODE_DOWN:
            docedit_move_cursor(editor, state->shift, 0, 1);
            break;
        case SDL_SCANCODE_LEFT:
            docedit_move_cursor(editor, state->shift, -1, 0);
            break;
        case SDL_SCANCODE_RIGHT:
            docedit_move_cursor(editor, state->shift, 1, 0);
            break;
        case SDL_SCANCODE_RETURN:
            docedit_insert(editor, "\n");
            break;
        case SDL_SCANCODE_TAB:
            for (int i = 0, col = editor->cursor.selection.from.column; i < (4 - col % 4); i++)
                docedit_insert(editor, " ");
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
