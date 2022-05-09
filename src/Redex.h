#ifndef R_REDEX_H
#define R_REDEX_H

#include "Buffer.h"
#include <stdbool.h>

struct {
    bool success;
    Cursor end;
}
typedef Redex_Match;

Redex_Match Redex_GetMatch(Buffer *buf, Cursor at, const char *seq);

#endif
