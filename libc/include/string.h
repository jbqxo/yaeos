#pragma once

#include <stddef.h>

void *memcpy(void * restrict dest, const void * restrict src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
char *strcpy(char * restrict dest, const char * restrict src);
char *strncpy(char * restrict dest, const char * restrict src, size_t n);
char *strcat(char * restrict dest, const char * restrict src);
char *strncat(char * restrict dest, const char * restrict src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
int strcoll(const char *s1, const char *s2);
size_t strxfrm(char * restrict s1, const char * restrict s2, size_t n);
void *memchr(const void *s, int c, size_t n);
char *strchr(const char *s, int c);
size_t strcspn(const char *s, const char *chs);
char *strpbrk(const char *s, const char *chs);
char *strrchr(const char *s, int ch);
size_t strspn(const char *s, const char *chs);
char *strstr(const char *s, const char *chs);
char *strtok(char * restrict s1, const char * restrict s2);
void *memset(void *str, int ch, size_t n);
char *strerror(int errnum);
size_t strlen(const char *str);
