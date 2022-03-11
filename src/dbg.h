void todo__(const char *file, int line, const char *function, const char *fmt, ...);
#define TODO(...) do {static int ONCE__ = 1; for (;ONCE__; ONCE__ = 0) todo__(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);} while(0)
