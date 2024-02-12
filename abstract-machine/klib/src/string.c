#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  size_t i;
  for (i = 0; s[i] != '\0'; ++i)
    len ++;
  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t i;
  for (i = 0; src[i] != '\0'; ++i)
    dst[i] = src[i];
  dst[i] = src[i];
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
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
  size_t i;
  char * str = (char *)s;
  for (i = 0; i < n; ++i)
    str[i] = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  int result = 0;
  unsigned char * str1 = (unsigned char *)s1;
  unsigned char * str2 = (unsigned char *)s2;
  size_t i;
  for (i = 0; i < n; ++i) {
    if ((result = str1[i] - str2[i]) != 0)
      break;
  }
  return result;
}

#endif
