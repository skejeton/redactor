#ifndef R_REDEX_H
#define R_REDEX_H

#include "Buffer.h"
#include "BufferTape.h"
#include <stdbool.h>

struct {
    bool success;
    BufferTape end;
}
typedef Redex_Match;

Redex_Match Redex_GetMatch(BufferTape tape, const char *seq);

#endif
