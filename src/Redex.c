#include "Redactor.h"
#include "Redex.h"
#include "Utf8.h"

static int GetSeqChar(const char **seq)
{
    int c = Utf8_NextVeryBad(seq);

    if (c == '%') {
        c = Utf8_NextVeryBad(seq);   
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


    for (int i = 0; (c = Utf8_NextVeryBad((const char **)&l.text)) && i < at.column; ++i)
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

        // NOTE: Handle range
        if (*seq == '-') {
            seq++;
            int startSeqChar = seqChar;
            int endSeqChar = GetSeqChar(&seq);
            if (bufChar >= startSeqChar && bufChar <= endSeqChar) {
                resultMatch.success = true;
                resultMatch.end = Buffer_MoveCursor(buf, at, 0, 1);
                break;
            }
        } else {
            if (seqChar == bufChar) {
                resultMatch.success = true;
                resultMatch.end = Buffer_MoveCursor(buf, at, 0, 1);
                break;
            }
        }
    }

    // Go until ']' (For multiple char matches)
    while (*seq && *seq != ']') {
        seq++;
    }


    // Skip ']'
    seq++;
    *endseq = seq;

    return resultMatch;
}

static Redex_Match MatchOneChar(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    int seqChar = GetSeqChar(&seq);
    *endseq = seq;
    if (seqChar == GetCharUnderCursor(buf, at)) {
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
