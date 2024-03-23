#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include "string.h"
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define ABS(num) ((num < 0) ? (-num) : (num))
#define TRANSFORM() while (number != 0) {\
                        int single_number = number % 10; \
                        buffer_reverse[count] = '0' + single_number;\
                        count ++; \
                        number /= 10; \
                    } 
void char_to_string(char *buffer, int c) {
    buffer[0] = c;
    buffer[1] = '\0';
}

void int_to_string(char *buffer, int number) {
    int count = 0;
    if (number == 0) {
        buffer[0] = '0';
        count ++;
    } else {
        char buffer_reverse[256];
    
        int is_minus = number < 0 ? 1 : 0;
    
        number = ABS(number);
        TRANSFORM();
        if (is_minus == 1) {
            buffer_reverse[count] = '-';
            count ++;
        }
    
        int i;
        for (i = 0; i < count; ++i)
            buffer[i] = buffer_reverse[count - 1 - i];
    }
    buffer[count] = '\0';
}

void uint_to_string(char *buffer, unsigned int number) {
    char buffer_reverse[256];
    int count = 0;
  
    TRANSFORM();
  
    int i;
    for (i = 0; i < count; ++i) {
        buffer[i] = buffer_reverse[count - 1 - i];
    }
    buffer[count] = '\0';
}

int parse_fmt(char *out, const char *fmt, va_list arg_list) {
    int is_persent_sign = 0;
    int i_out = 0;
    for (int i = 0; fmt[i] != '\0'; ++ i) {
        if (fmt[i] == '%')
            is_persent_sign = 1;
        else if (is_persent_sign == 1) {
            char buffer[256];
            char *temp_str;
            switch (fmt[i]) {
                case 'd':
                    int_to_string(buffer, va_arg(arg_list, int));
                    strcpy(out + i_out, buffer);
                    i_out += strlen(buffer);
                    break;
                case 'u':
                    uint_to_string(buffer, va_arg(arg_list, unsigned int));
                    strcpy(out + i_out, buffer);
                    i_out += strlen(buffer);
                    break;
                case 's':
                    temp_str = va_arg(arg_list, char *);
                    strcpy(out + i_out, temp_str);
                    i_out += strlen(temp_str);
                    break;
                case 'c':
                    char_to_string(buffer, va_arg(arg_list, int));
                    strcpy(out + i_out, buffer);
                    i_out += strlen(buffer);
                default:
                    panic("Bad %... in sprintf");
            }
            is_persent_sign = 0;
        } else {
            out[i_out] = fmt[i];
            i_out ++;
        }
    }
    out[i_out] = '\0';
    return i_out;
}

int vprintf(const char *fmt, va_list arg_list) {
    char out[1024];
    int counter = 0;

    counter = parse_fmt(out, fmt, arg_list);

    int i = 0;

    while (out[i] != '\0') {
        putch(out[i]);
        i ++;
    }

    return counter;
}

int printf(const char *fmt, ...) {
    int counter = 0;

    va_list arg_list;
    va_start(arg_list, fmt);

    counter = vprintf(fmt, arg_list);

    va_end(arg_list);

    return counter;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    int len_success = parse_fmt(out, fmt, ap);
    return len_success;
}

int sprintf(char *out, const char *fmt, ...) {
  int len_success = 0;
  va_list arg_list;
  va_start(arg_list, fmt);

  len_success = vsprintf(out, fmt, arg_list);

  va_end(arg_list);
  return len_success;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
