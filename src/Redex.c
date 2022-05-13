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

static int In_GetCharUnderCursor(Buffer *buf, Cursor at)
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
    Redex_Match resultMatch = {.end = at};

    // Skip '['
    seq++;
    bool negate = *seq == '^';
    if (negate) {
        seq++;
    }

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
                break;
            }
        } else {
            if (seqChar == bufChar) {
                resultMatch.success = true;
                break;
            }
        }
    }

    // Go until ']' (For multiple char matches)
    while (*seq && *seq != ']') {
        In_GetSeqChar(&seq);
    }

    // Skip ']'
    seq++;
    *endseq = seq;

    resultMatch.success = resultMatch.success != negate;

    if (resultMatch.success) {
        resultMatch.end = Buffer_MoveCursor(buf, at, 0, 1);
    }

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

static Redex_Match In_MatchGroup(Buffer *buf, Cursor at, const char **endseq, const char *seq);
static Redex_Match In_MatchBasic(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    Redex_Match match;
    switch (*seq) {
    case '[':
        match = In_MatchAnyChar(buf, at, &seq, seq);
        break;
    case '(':
        match = In_MatchGroup(buf, at, &seq, seq);
        break;
    default:
        match = In_MatchOneChar(buf, at, &seq, seq);
        break;
    }
    *endseq = seq;
    return match;
}

static Redex_Match In_MatchMany(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    const char *start = seq;
    Redex_Match match = In_MatchBasic(buf, at, &seq, start);

    switch (*seq) {
    case '+':
        for (Redex_Match candidate = match; candidate.success && Buffer_CompareCursor(candidate.end, at) > 0; candidate = In_MatchBasic(buf, at, &seq, start)) {
            match = candidate;
            at = match.end;
        }
        seq++;
        break;
    case '*':
        // TODO: This check may be unnecessary if we returned back to the start on failed matches
        if (!match.success) {
            match.success = true;
            match.end = at;
        }
        else {
            for (Redex_Match candidate = match; candidate.success && Buffer_CompareCursor(candidate.end, at) > 0; candidate = In_MatchBasic(buf, at, &seq, start)) {
                match = candidate;
                at = match.end;
            }
        }
        seq++;
        break;
    case '?':
        if (!match.success) {
            match.success = true;
            match.end = at;
        }
        seq++;
        break;
    default:
        break;
    }

    *endseq = seq;
    return match;
}

static Redex_Match In_MatchGroup(Buffer *buf, Cursor at, const char **endseq, const char *seq)
{
    seq++;
    bool success = true;

    while (*seq && *seq != ')') {
        Redex_Match match = In_MatchMany(buf, at, &seq, seq);

        if (!match.success) {
            success = false;
            break;
        }

        at = match.end;
    }

    // Skip remaining regex syntax...
    // This is a pretty bad way to do it since it's going to try to match with the buffer, we don't want it as it's pretty slow...
    // There's 2 solutions I can think of right now:
    // Skippers * will skip whatever the regex syntax.
    //   Honestly I'm not a fan of this idea, since we will have to mirror parsing code which may lead to bugs and inconsistencies.
    //
    // Parser * will parse the regex, and then execute it. That makes it clear where the bounds are.
    //   The disadvantage of a parser is that the parse time and storage of the parser strings may need many sparse memory allocations.
    //   I will definitely try parsers, unless I find a better solution.
    //
    // We can't just skip characters until ')', because it will mess up nesting and escaping.
    while (*seq && *seq != ')') {
        In_MatchMany(buf, at, &seq, seq);
    }

    // Skip ')'
    if (*seq) {
        seq++;
    }

    *endseq = seq;
    return (Redex_Match){.success = success, .end = at};
}

// TODO: This is almost an identical copy of In_MatchGroup,
//       the only difference is that In_MatchGroup handles parentheses.
//       There could be way to avoid duplication here.
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
