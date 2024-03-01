#include <stdint.h>

// aimed for the most irritating code possible

size_t strnlen(const char *s, size_t maxlen) {
    const char *es = s;
    while (*es && maxlen--) es++;
    return es - s;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n-- && (*d++ = *src++));
    if (~n) while (n--) *d++ = 0;
    return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (*d) d++;
    while (n-- && (*d++ = *src++));
    *d = 0;
    return dest;
}

char *strstr(const char *haystack, const char *needle) {
    while (*haystack) {
        const char *h = haystack++;
        const char *n = needle;
        while (*h++ == *n++) if (!*n) return (char *) haystack;
    }
    return NULL;
}
