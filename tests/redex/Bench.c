#include "Buffer.h"
#include "BufferDraw.h"
#include "HighlightSets.h"
#include "tests/test.h"
#include "Util.h"
#include "Highlight.h"

void Bench_Redex_Main()
{
    FILE *f = fopen("tests/TestCParsingRedexPerformance.txt", "r");
    char *str = Util_ReadFileStr(f);
    fclose(f);
    Buffer buf = Buffer_InitFromString(str);
    Highlight_Set set = HighlightSets_Compile(&HighlightSets_C);
    BufferDrawSegments segments = { 0 };
    Info("Warning: this test also relies on performance of highlight, and should do all highlighting from scratch every time.");

    Benchmark("Recursive Matcher", 50) {
        Highlight_HighlightBuffer(&buf, &set, &segments);
    }

    free(str);
    free(segments.segments);
    Highlight_HighlightSetDeinit(&set);
    Buffer_Deinit(&buf);
}