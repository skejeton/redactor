#include "Redactor.h"
#include "Redex.h"
#include "Unicode.h"

static int GetSeqChar(const char **seq)
{
    int c = Uni_Utf8_NextVeryBad(seq);

    if (c == '%') {
        c = Uni_Utf8_NextVeryBad(seq);   
    }

    return c;
}

int GetCharUnderCursor(Buffer *buf, Cursor at)
{
    if (at.line >= buf->lines_len) {
        return 0;
    }

    Line l = buf->lines[at.line];
    int c;

    for (int i = 0; (c = Uni_Utf8_NextVeryBad((const char **)&l.text) && i < at.column); ++i)
        ;

    return c ? c : '\n';
}

static Redex_Match MatchAnyChar(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    Redex_Match resultMatch = {0};

    // Skip '['
    seq++;

    while (*seq && *seq != ']') {
        int seqChar = GetSeqChar(&seq);
        int bufChar = GetCharUnderCursor(buf, at);
        Buffer_MoveCursor(buf, at, 0, 1);

        if (seqChar == bufChar) {
            resultMatch.success = true;
            resultMatch.end = at;
            break;
        }
    }


    // Skip ']'
    seq++;
    *endseq = seq;

    return resultMatch;
}

static Redex_Match MatchOneChar(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    int seqChar = GetSeqChar(&seq);
    if (seqChar != GetCharUnderCursor(buf, at)) {
        return (Redex_Match){.end = Buffer_MoveCursor(buf, at, 0, 1), .success = true};
    } else {
        return (Redex_Match){.success = false};
    }
}

Redex_Match Redex_GetMatch(Buffer *buf, Cursor at, const char *seq)
{
    while (*seq) {
        Redex_Match match;

        if (*seq == '[') {
            match = MatchAnyChar(buf, at, &seq, seq);
        } else {
            match = MatchOneChar(buf, at, &seq, seq);
        }

        if (!match.success) {
            return (Redex_Match){.success = false, .end = at};
        }
        at = match.end;
    }

    return (Redex_Match){.success = true, .end = at};
}
