#include <lib/string.h>
#include <mm/heap.h>

size_t strlen(const char *s) {
    size_t len = 0;
    while (*s) {
        s++;
        len++;
    }
    return len;
}

char *strcat(char *dest, const char *src) {
    char *d = dest + strlen(dest);
    strcpy(d, src);
    return dest;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == c) {
            return (char *)s;
        }
        s++;
    }
    if (*s == c) {
        return (char *)s;
    }
    return nullptr;
}
char *strrchr(const char *s, int c) {
    const char *r = s + strlen(s);
    while (r > s) {
        if (*r == c) {
            return (char *)r;
        }
        r--;
    }
    return nullptr;
}

char *strstr(const char *s1, const char *s2) {
    if (*s2 == '\0') {
        return (char *)s1;
    }
    size_t s2_len = strlen(s2);
    while (*s1) {
        if (*s1 == *s2 && strncmp(s1, s2, s2_len) == 0) {
            return (char *)s1;
        }
        s1++;
    }
    return nullptr;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }
    return *s1 - *s2;
}
int strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) {
        return 0;
    }
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            return *s1 - *s2;
        }
        s1++;
        s2++;
        n--;
        if (n == 0) {
            return 0;
        }
    }
    return *s1 - *s2;
}

char *strcpy(char *dest, const char *src) {
    const char *s = src;
    char *d = dest;
    while (*s) {
        *d = *s;
        d++;
        s++;
    }
    *d = '\0';
    return dest;
}
char *strncpy(char *dest, const char *src, size_t n) {
    if (n == 0) {
        return dest;
    }
    char *d = dest;
    const char *s = src;
    while (*s && n > 0) {
        *d = *s;
        d++;
        s++;
        n--;
    }
    if (n > 0) {
        memset(d, 0, n);
    }
    return d;
}

char *strdup(const char *s) {
    size_t len = strlen(s);
    size_t cap = len + 1;

    char *duped = kmalloc(cap);
    memcpy(duped, s, cap);

    return duped;
}
char *strndup(const char *s, size_t n) {
    size_t cap = n + 1;
    char *duped = kmalloc(cap);

    char *d = duped;
    while (*s && n > 0) {
        *d = *s;
        d++;
        s++;
        n--;
    }
    *d = '\0';
    return duped;
}
size_t strspn(const char *s, const char *accept) {
    size_t count = 0;
    while (*s) {
        if (strchr(accept, *s) != nullptr) {
            count++;
        } else {
            return count;
        }
        s++;
    }
    return count;
}
char *strpbrk(const char *s, const char *accept) {
    while (*s) {
        if (strchr(accept, *s) != nullptr) {
            return (char *)s;
        }
        s++;
    }
    return nullptr;
}

// Based off https://github.com/walac/glibc/blob/master/string/strtok.c
// Too lazy to write this
static char *olds;
char *strtok(char *s, const char *delim) {
    char *token;
    if (s == nullptr) {
        s = olds;
    }

    // Remove leading
    s += strspn(s, delim);
    if (*s == '\0') {
        olds = s;
        return nullptr;
    }

    // Current token
    token = s;
    s = strpbrk(token, delim);
    if (s == nullptr) {
        // End of tokens
        olds = strchr(token, '\0');
    } else {
        // Break at delim
        *s = '\0';
        olds = s + 1;
    }
    // Return token
    return token;
}
