#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include "string.h"
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define ABS(num) ((num < 0) ? (-num) : (num))
#define TRANSFORM_10() while (number != 0) {\
                        int single_number = number % 10; \
                        buffer_reverse[count] = '0' + single_number;\
                        count ++; \
                        number /= 10; \
                    } 

#define TRANSFORM_16() while (number != 0) { \
    int single_number = number % 16; \
    buffer_reverse[count] = dec_to_hex(single_number);\
    count ++; \
    number /= 16; \
}

#define ISDATETYPECHAR(c) c == 'd' ||\
                          c == 'u' ||\
                          c == 'x' ||\
                          c == 's' ||\
                          c == 'c'

char dec_to_hex(int num) {
    char res;
    switch(num) {
        case 10:
            res = 'a'; break;
        case 11:
            res = 'b'; break;
        case 12:
            res = 'c'; break;
        case 13:
            res = 'd'; break;
        case 14:
            res = 'e'; break;
        case 15:
            res = 'f'; break;
        default:
            res = '0' + num; break;
    }
    return res;
}

/* void char_to_string(char *buffer, int c) {*/
/*     buffer[0] = c;*/
/*     buffer[1] = '\0';*/
/* }*/

void int_to_string(char *buffer, int number) {
    int count = 0;
    if (number == 0) {
        buffer[0] = '0';
        count ++;
    } else {
        char buffer_reverse[256];
    
        int is_minus = number < 0 ? 1 : 0;
    
        number = ABS(number);
        TRANSFORM_10();
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

void uint_to_string(char *buffer, unsigned int number, int base) {
    char buffer_reverse[256];
    int count = 0;
  
    if (base == 10)
        TRANSFORM_10();
    if (base == 16)
        TRANSFORM_16();
  
    int i;
    for (i = 0; i < count; ++i) {
        buffer[i] = buffer_reverse[count - 1 - i];
    }
    buffer[count] = '\0';
}

void parse_placeholder(char *out_buffer, char *in_buffer, va_list *p_arg_list) {
    int pad_zero = 0, width = 0;
    char data[256];
    int i = 0;
    while (in_buffer[i] != '\0'){
        if (in_buffer[i] == '0') {
            pad_zero = 1;
        } else if (in_buffer[i] >= '1' && in_buffer[i] <= '9') {
            width = in_buffer[i] - '0';
        } else {
            switch (in_buffer[i]) {
                case 'd':
                    int_to_string(data, va_arg(*p_arg_list, int));
                    break;
                case 'u':
                    uint_to_string(data, va_arg(*p_arg_list, unsigned int), 10);
                    break;
                case 'x':
                    uint_to_string(data, va_arg(*p_arg_list, unsigned int), 16);
                    break;
                case 's':
                    strcpy(data, va_arg(*p_arg_list, char *));
                    break;
                case 'c':
                    data[0] = va_arg(*p_arg_list, int);
                    data[1] = '\0';
                    break;
                default:
                    putch(in_buffer[i]);
                    putch('\n');
                    panic("Error data type in parse_placeholder()!\n");
                    break;
            }
        }
        i ++;
    }

    char padchar = ' ';
    if (pad_zero) {
        padchar = '0';
    }

    int datalen = strlen(data);
    int padlen = width > datalen ? width - datalen : 0;
    memset(out_buffer, padchar, padlen);
    strcpy(out_buffer + padlen, data);
}

int parse_fmt(char *out, const char *fmt, va_list arg_list) {
    int is_placeholder = 0;
    int i_out = 0;

    for (int i = 0; fmt[i] != '\0'; ++ i) {
        if (fmt[i] == '%' && is_placeholder == 0) {
            is_placeholder = 1;
        } else if (fmt[i] == '%' && is_placeholder == 1) {
            out[i_out] = '%';
            i_out ++;
            is_placeholder = 0;
        } else if (fmt[i] != '%' && is_placeholder == 0) {
            out[i_out] = fmt[i];
            i_out ++;
        } else if (fmt[i] != '%' && is_placeholder == 1) {
            char buffer[256];
            int i_buffer = 0;
            while (!(ISDATETYPECHAR(fmt[i]))) {
                if (fmt[i] == '\0')
                    panic("The placeholder don't have date type char.");
                buffer[i_buffer] = fmt[i];
                i_buffer ++;
                i ++;
            }
            buffer[i_buffer++] = fmt[i];
            buffer[i_buffer] = '\0';

            char out_buffer[256];
            parse_placeholder(out_buffer, buffer, &arg_list);

            strcpy(out + i_out, out_buffer);
            i_out += strlen(out_buffer);

            is_placeholder = 0;
        }
    }
    out[i_out] = '\0';
    return i_out;
}

int vprintf(const char *fmt, va_list arg_list) {
    int len_success = 0;

    char out[4096];
    len_success = parse_fmt(out, fmt, arg_list);

    int i = 0;

    while (out[i] != '\0') {
        putch(out[i]);
        i ++;
    }

    return len_success;
}

int printf(const char *fmt, ...) {
    int len_success = 0;

    va_list arg_list;
    va_start(arg_list, fmt);

    len_success= vprintf(fmt, arg_list);

    va_end(arg_list);

    return len_success;
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
