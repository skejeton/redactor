#ifndef REDACTOR_UTF8_H
#define REDACTOR_UTF8_H
static inline int utf8_get(const char **const restrict s_, int *restrict max)
{
    const static int class[32] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5 };
    unsigned char *s = (unsigned char*)*s_;
    int c = 0;
    int cl = class[*s>>3];
    if (cl > *max) {
        return 0;
    }
    *max -= cl;
    switch (cl) {
        case 1:
            c = *s;
            break;
        case 2:
            c = (*s&0x1f)<<6;
            c |= (*++s&0x3f);
            break;
        case 3:
            c = (*s&0xf)<<12;
            c |= (*++s&0x3f)<<6;
            c |= (*++s&0x3f);
            break;
        case 4:
            c = (*s&0x7)<<18;
            c |= (*++s&0x3f)<<12;
            c |= (*++s&0x3f)<<6;
            c |= (*++s&0x3f);
            break;
    }
    *s_ = (char*)(s+!!c); 
    return c;
}
#endif