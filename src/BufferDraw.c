#include "BufferDraw.h"
#include "Redactor.h"
#include <assert.h>

struct DataMarker {
    Line line;
    size_t lineNo;
    int column;
};

void BufferDraw_InvalidateSegments(BufferDrawSegments *seg)
{
    free(seg->segments);
    seg->segments = 0;
    seg->segments_len = 0;
}

void BufferDraw_InsertSegment(BufferDrawSegments *seg, size_t line, int column, size_t offset, SDL_Color color)
{
    if (seg->segments_len % 512 == 0) {
        seg->segments = realloc(seg->segments, sizeof(*seg->segments) * (seg->segments_len+512));
    }

    seg->segments[seg->segments_len++] = (BufferDrawSegment){.line = line, .column = column, .offset = offset, .color = color};
}

void BufferDraw_DrawBuffer(Redactor *rs, Buffer *buf, BufferDrawSegments *seg)
{
    assert(buf->lines_len > 0);

    SDL_Point position = {rs->render_scroll.x, rs->render_scroll.y};
    struct DataMarker start = { buf->lines[0], 0, 0 };

    for (size_t i = 0; i < seg->segments_len; ++i) {
        const BufferDrawSegment segment = seg->segments[i];
        // TODO: Don't render out of bounds text

        char tmp = buf->lines[segment.line].text[segment.offset];
        buf->lines[segment.line].text[segment.offset] = 0;

        while (start.lineNo < segment.line) {
            Redactor_DrawText(rs, segment.color, start.line.text, 0, position.x, position.y, start.column);
            position.x = rs->render_scroll.x;
            position.y += rs->render_font_height;
            start.column = 0;
            start.line = buf->lines[++start.lineNo];
        }

        position.x = Redactor_DrawText(rs, segment.color, start.line.text, 0, position.x, position.y, start.column).x;

        start.line.text = buf->lines[segment.line].text+segment.offset;
        start.column = segment.column;
        buf->lines[segment.line].text[segment.offset] = tmp;
    }
}
