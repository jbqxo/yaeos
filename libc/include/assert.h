#undef assert

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG
#define assert(exp) ((void)0)
#else
// TODO(Maxim Lyapin): Well. Right now assertion isn't very good. Make it good.
#define assert(exp) ((exp) && ((*(int*)0x0 = 0), 1))
#endif

// TODO(Maxim Lyapin): Mark as C11 or newer only
#define static_assert(exp, str) _Static_assert(exp, str)

#ifdef __cplusplus
}
#endif
