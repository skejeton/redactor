#include "../test.h"
#include "src/BufferTape.h"

void Test_BufferTape_Main()
{
    {
        Buffer buf = Buffer_InitFromString("Txt\n");
        BufferTape tape = BufferTape_Init(&buf);
        Expect(tape.cursor.line == 0 && tape.cursor.column == 0);
        Expect(BufferTape_Next(&tape) == 'T');
        Expect(tape.cursor.line == 0 && tape.cursor.column == 1);
        Expect(BufferTape_Next(&tape) == 'x');
        Expect(tape.cursor.line == 0 && tape.cursor.column == 2);
        Expect(BufferTape_Next(&tape) == 't');
        Expect(tape.cursor.line == 0 && tape.cursor.column == 3);
        Expect(BufferTape_Next(&tape) == '\n');
        Info("%d %d", tape.cursor.line, tape.cursor.column);
        Expect(tape.cursor.line == 1 && tape.cursor.column == 0);
        Buffer_Deinit(&buf);
    }
    {
        Buffer buf = Buffer_InitFromString("\n");
        BufferTape tape = BufferTape_Init(&buf);
        Expect(tape.cursor.line == 0 && tape.cursor.column == 0);
        Expect(BufferTape_Next(&tape) == '\n');
        Info("%d %d", tape.cursor.line, tape.cursor.column);
        Expect(tape.cursor.line == 1 && tape.cursor.column == 0);
        Buffer_Deinit(&buf);
    }
    {
        Buffer buf = Buffer_InitFromString("Testこ\n123");
        BufferTape tape = BufferTape_Init(&buf);
        Expect(tape.cursor.line == 0 && tape.cursor.column == 0);
        Expect(BufferTape_Next(&tape) == 'T');
        Expect(tape.cursor.line == 0 && tape.cursor.column == 1);
        Expect(BufferTape_Next(&tape) == 'e');
        Expect(tape.cursor.line == 0 && tape.cursor.column == 2);
        Expect(BufferTape_Next(&tape) == 's');
        Expect(tape.cursor.line == 0 && tape.cursor.column == 3);
        Expect(BufferTape_Next(&tape) == 't');
        Expect(tape.cursor.line == 0 && tape.cursor.column == 4);
        Expect(BufferTape_Next(&tape) == L'こ');
        Info("%d %d", tape.cursor.line, tape.cursor.column);
        Expect(tape.cursor.line == 0 && tape.cursor.column == 5);
        Expect(BufferTape_Next(&tape) == '\n');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 0);
        Expect(BufferTape_Next(&tape) == '1');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 1);
        Expect(BufferTape_Next(&tape) == '2');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 2);
        Expect(BufferTape_Next(&tape) == '3');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 3);
        Expect(BufferTape_Next(&tape) == '\0');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 3);
        Buffer_Deinit(&buf);
    }
    {
        Buffer buf = Buffer_InitFromString("GARBAGE TEXT ここここ\nGARBAGEこTestこ\n123");
        BufferTape tape = BufferTape_InitAt(&buf, (Cursor){1, 8});
        Expect(tape.cursor.line == 1 && tape.cursor.column == 8);
        Expect(BufferTape_Next(&tape) == 'T');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 9);
        Expect(BufferTape_Next(&tape) == 'e');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 10);
        Expect(BufferTape_Next(&tape) == 's');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 11);
        Expect(BufferTape_Next(&tape) == 't');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 12);
        Expect(BufferTape_Next(&tape) == L'こ');
        Expect(tape.cursor.line == 1 && tape.cursor.column == 13);
        Expect(BufferTape_Next(&tape) == '\n');
        Expect(tape.cursor.line == 2 && tape.cursor.column == 0);
        Expect(BufferTape_Next(&tape) == '1');
        Expect(tape.cursor.line == 2 && tape.cursor.column == 1);
        Expect(BufferTape_Next(&tape) == '2');
        Expect(tape.cursor.line == 2 && tape.cursor.column == 2);
        Expect(BufferTape_Next(&tape) == '3');
        Expect(tape.cursor.line == 2 && tape.cursor.column == 3);
        Expect(BufferTape_Next(&tape) == '\0');
        Expect(tape.cursor.line == 2 && tape.cursor.column == 3);
        Buffer_Deinit(&buf);
    }
}
