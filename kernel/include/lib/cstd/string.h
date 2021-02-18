#ifndef _LIB_STRING_H
#define _LIB_STRING_H

#include <stddef.h>

void *kmemcpy(void *restrict dest, const void *restrict src, size_t n);
void *kmemmove(void *dest, const void *src, size_t n);
char *kstrcpy(char *restrict dest, const char *restrict src);
char *kstrncpy(char *restrict dest, const char *restrict src, size_t n);
char *kstrcat(char *restrict dest, const char *restrict src);
char *kstrncat(char *restrict dest, const char *restrict src, size_t n);
int kmemcmp(const void *s1, const void *s2, size_t n);
int kstrcmp(const char *s1, const char *s2);
int kstrncmp(const char *s1, const char *s2, size_t n);
int kstrcoll(const char *s1, const char *s2);
size_t kstrxfrm(char *restrict s1, const char *restrict s2, size_t n);
void *kmemchr(const void *s, int c, size_t n);
char *kstrchr(char *s, int c);
size_t kstrcspn(const char *s, const char *chs);
char *kstrpbrk(const char *s, const char *chs);
char *kstrrchr(const char *s, int ch);
size_t kstrspn(const char *s, const char *chs);
char *kstrstr(char *s, const char *chs);
char *kstrtok(char *restrict s1, const char *restrict s2);
void *kmemset(void *str, int ch, size_t n);
char *kstrerror(int errnum);
size_t kstrlen(const char *str);

#endif //  _LIB_STRING_H
