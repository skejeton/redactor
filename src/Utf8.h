#include <stdint.h>
#include <stddef.h>

static inline int Utf8_RuneLen(const char *s_)
{
    const unsigned char *s = (const unsigned char *)s_;
    const static int clas[32] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5 };
    return *s == 0 ? 0 : clas[*s>>3];
}

static inline size_t Utf8_Fetch(uint32_t *out, const char *s_) {
	const unsigned char *s = (const unsigned char*)s_;
	if ((*s & 0xC0) != 0xC0) {
		*out = *s;
		return *s > 0;
	}

	const static size_t clas[32] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5};
	size_t cl = clas[*s>>3];

	for (size_t i = 1; i < cl; ++i) {
		if ((s[i] & 0xC0) == 0xC0 || (s[i] & 0x80) == 0) {
			*out = s[0];
			return 1;
		}
	}

	switch (cl) {
        // Case 0 and 1 will not happen here as they are handled in first if statement.
		case 2: *out = ((s[0]&0x1f)<<6) | (s[1]&0x3f); break;
		case 3: *out = ((s[0]&0xf)<<12) | ((s[1]&0x3f)<<6) | (s[2]&0x3f); break;
		case 4: *out = ((s[0]&0x7)<<18) | ((s[1]&0x3f)<<12) | ((s[2]&0x3f)<<6) | (s[3]&0x3f); break;
		case 5: *out = 0; return 0; // This is an errorneous case, 5 isn't defined in UTF-8
	}

	return cl;
}

static inline int Utf8_NextVeryBad(const char **s)
{
    uint32_t out;
    *s += Utf8_Fetch(&out, *s);
    return out;
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
