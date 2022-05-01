#ifndef R_MEM2_H
#define R_MEM2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void* memcpy2(void* dest, const void* src, size_t count) {
    void *ret = memcpy(dest, src, count);
    printf("memcpy2(%p, %p, %zu) -> %p\n", dest, src, count, ret);
    return ret;
}

#endif