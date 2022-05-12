#include "Redactor.h"
#include "Redex.h"
#include "Utf8.h"

static int In_GetSeqChar(const char **seq)
{
    int c = Utf8_NextVeryBad(seq);

    if (c == '%') {
        c = Utf8_NextVeryBad(seq);   
        switch (c) {
        case 't': return '\t';
        case 'n': return '\n';
        case 's': return ' ';
        }
    }

    return c;
}

int In_GetCharUnderCursor(Buffer *buf, Cursor at)
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

static Redex_Match In_MatchAnyChar(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    Redex_Match resultMatch = {0};

    // Skip '['
    seq++;

    while (*seq && *seq != ']') {
        int seqChar = In_GetSeqChar(&seq);
        int bufChar = In_GetCharUnderCursor(buf, at);

        // NOTE: Handle range
        if (*seq == '-') {
            seq++;
            int startSeqChar = seqChar;
            int endSeqChar = In_GetSeqChar(&seq);
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

static Redex_Match In_MatchOneChar(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    int seqChar = In_GetSeqChar(&seq);
    *endseq = seq;
    if (seqChar == In_GetCharUnderCursor(buf, at)) {
        return (Redex_Match){.end = Buffer_MoveCursor(buf, at, 0, 1), .success = true};
    } else {
        return (Redex_Match){.success = false};
    }
}

Redex_Match In_MatchBasic(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    Redex_Match match;
    if (*seq == '[') {
        match = In_MatchAnyChar(buf, at, &seq, seq);
    } else {
        match = In_MatchOneChar(buf, at, &seq, seq);
    }
    *endseq = seq;
    return match;
}

Redex_Match In_MatchMany(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    const char *start = seq;
    Redex_Match match = In_MatchBasic(buf, at, &seq, start);

    if (*seq == '+') {
        for (Redex_Match candidate = match; candidate.success; candidate = In_MatchBasic(buf, at, &seq, start)) {
            match = candidate;
            at = match.end;
        }
        seq++;
    } else if (*seq == '*') {
        match.success = true;
        for (Redex_Match candidate = match; candidate.success; candidate = In_MatchBasic(buf, at, &seq, start)) {
            match = candidate;
            at = match.end;
        }
        seq++;
    }

    *endseq = seq;
    return match;
}

Redex_Match Redex_GetMatch(Buffer *buf, Cursor at, const char *seq)
{
    while (*seq) {
        Redex_Match match = In_MatchMany(buf, at, &seq, seq);

        if (!match.success) {
            return (Redex_Match){.success = false, .end = at};
        }

        at = match.end;
    }

    return (Redex_Match){.success = true, .end = at};
}
