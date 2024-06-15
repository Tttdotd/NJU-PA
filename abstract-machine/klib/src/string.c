#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    if (s == NULL) {
        return 0;
    }
    size_t len = 0;
    size_t i;
    for (i = 0; s[i] != '\0'; ++i) {
        len ++;
    }
    return len;
}

char *strcpy(char *dst, const char *src) {
    size_t i;
    for (i = 0; src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }
    dst[i] = src[i];
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    int j = 0;
    for (int i = 0; i < n; i ++) {
        if (src[i] == '\0')
            dst[i] = '\0';
        else {
            dst[i] = src[i];
            j ++;
        }
    }
    return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dst_len = strlen(dst);
  size_t i;
  for ( i = 0; src[i] != '\0'; ++i) {
    dst[i+dst_len] = src[i];
  }
  dst[i+dst_len] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
    while ( *s1 && *s2 && *s1 == *s2) {
        s1 ++;
        s2 ++;
    }
    return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    size_t count = 0;
    while ( *s1 && *s2 && (*s1 == *s2) && count != n) {
        s1 ++;
        s2 ++;
    }
    return *s1 - *s2;
}

void *memset(void *s, int c, size_t n) {
    const unsigned char uc = c;
    unsigned char * p;
    for (p = s; n > 0; ++p, --n) {
        *p = uc;
    }
    return s;
}

void *memmove(void *dst, const void *src, size_t n) {
    char *d = (char *)dst;
    char *s = (char *)src;
    if (dst > src) {
        for (int i = n; i >= 0; i --) {
            d[i] = s[i];
        }
    } else {
        for (int i = 0; i < n; i ++) {
            d[i] = s[i];
        }
    }
    return (void *)d;
}

void *memcpy(void *out, const void *in, size_t n) {
    char *dest = (char *)out;
    char *src = (char *)in;
    
    //printf("memcpy n: %d\n", n);
    for (int i = 0; i < n; i ++) {
        dest[i] = src[i];
    }
    return (void*)dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    int result = 0;
    unsigned char * str1 = (unsigned char *)s1;
    unsigned char * str2 = (unsigned char *)s2;
    size_t i;
    for (i = 0; i < n; ++i) {
        if ((result = str1[i] - str2[i]) != 0) {
            break;
        }
    }
    return result;
}

#endif
