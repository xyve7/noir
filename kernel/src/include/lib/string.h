#include <stddef.h>
#include <stdint.h>

extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memmove(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);
extern void *memchr(const void *s, int c, size_t n);

size_t strlen(const char *s);

char *strcat(char *dest, const char *src);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *s1, const char *s2);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

char *strdup(const char *s);
char *strndup(const char *s, size_t n);

char *strtok(char *s, const char *delim);

size_t strspn(const char *s, const char *accept);
char *strpbrk(const char *s1, const char *s2);

size_t strcnt(const char *s, int ch);
