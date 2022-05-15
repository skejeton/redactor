static inline int Utf8_RuneLen(const char *s_)
{
    const unsigned char *s = (const unsigned char *)s_;
    const static int clas[32] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5 };
    return *s == 0 ? 0 : clas[*s>>3];
}

static inline int Utf8_GetVeryBad(const char *s)
{
    const static int clas[32] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5 };
    int c = 0;
    int cl = clas[*s>>3];

    for (int i = 0; i < cl; ++i) {
        // NOTE: Remove garbage data in between null terminator
        if (s[i] == 0) {
            cl = 1;
            break;
        }
    }

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

    return c;
}

static inline int Utf8_NextVeryBad(const char **s_)
{
    const static int clas[32] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5 };
    unsigned char *s = (unsigned char*)*s_;
    int c = 0;
    int cl = clas[*s>>3];

    for (int i = 0; i < cl; ++i) {
        // NOTE: Remove garbage data in between null terminator
        if (s[i] == 0) {
            cl = 1;
            break;
        }
    }

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

static inline const char *Utf8_Strchr(const char *s, int ch)
{
    int c;
    const char *p = s;
    while ((c = Utf8_NextVeryBad(&s))) {
        if (c == ch) {
            return p;       
        }
        p = s;
    }
    return 0;
}

// NOTE: You probably don't want to use this
static inline int Utf8_Strlen(const char *s)
{
    int i = 0;
    while (Utf8_NextVeryBad(&s))
        ++i;
    return i;
}
