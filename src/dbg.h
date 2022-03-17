#define ATTR_FMT__(a,b)
#if defined __has_attribute
#if __has_attribute(format)
#undef ATTR_FMT__
#define ATTR_FMT__(a,b) __attribute__((format(printf, a, b)))
#endif
#endif

ATTR_FMT__(4, 5) void todo__(const char *file, int line, const char *function, const char *fmt, ...);
ATTR_FMT__(6, 7) int assert__(const char *file, int line, const char *function, const char *s, _Bool x, const char *fmt, ...);
#define TODO(...) do {static int ONCE__ = 1; for (;ONCE__; ONCE__ = 0) todo__(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);} while(0)
#define ASSERT(x,...) (assert__(__FILE__, __LINE__, __FUNCTION__, (#x), (x), __VA_ARGS__) && (abort(),1))

#undef ATTR_FMT__
