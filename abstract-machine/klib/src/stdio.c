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

void number_replace(char type, char *modifiers, char *buffer, va_list *arg_list) {
    char buffer_value[256];
    if (type == 'd') {
        int_to_string(buffer_value, va_arg(*arg_list, int));
    }
    if (type == 'u') {
        uint_to_string(buffer_value, va_arg(*arg_list, unsigned int), 10);
    }
    if (type == 'x') {
        uint_to_string(buffer_value, va_arg(*arg_list, unsigned int), 16);
    }

    int i_buffer = 0;

    if (modifiers[0] == '\0') {
        strcpy(buffer, buffer_value);
    } else if (modifiers[1] == '\0') {
        int num_blank = modifiers[0] - '0' - strlen(buffer_value);
        for (int i = 0; i < num_blank; i ++) {
            buffer[i_buffer] = ' ';
            i_buffer ++;
        }
        strcpy(buffer + i_buffer, buffer_value);
    } else if (modifiers[2] == '\0') {
        int num_zero = modifiers[1] - '0' - strlen(buffer_value);
        for (int i = 0; i < num_zero; i ++) {
            buffer[i_buffer] = '0';
            i_buffer ++;
        }
        strcpy(buffer + i_buffer, buffer_value);
    }
}

void string_replace(char *modifiers, char *buffer, char *string) {
    char buffer_string[256];
    strcpy(buffer_string, string);
    int i_buffer = 0;
    if (modifiers[0] == '\0') {
        strcpy(buffer, string);
    } else if (modifiers[1] == '\0') {
        int num_blank = modifiers[0] - '0' - strlen(buffer_string);
        for (int i = 0; i < num_blank; i ++) {
            buffer[i_buffer] = ' ';
            i_buffer ++;
        }
        strcpy(buffer + i_buffer, buffer_string);
    }
}

void char_replace(char *modifiers, char *buffer, char c) {
    int i_buffer = 0;
    if (modifiers[0] == '\0') {
        buffer[i_buffer] = c;
    } else if (modifiers[1] == '\0') {
        int num_blank = modifiers[0] - '0' - 1;
        for (int i = 0; i < num_blank; i ++) {
            buffer[i_buffer] = '0';
            i_buffer ++;
        }
        buffer[i_buffer] = c;
    }
}

int parse_fmt(char *out, const char *fmt, va_list arg_list) {
    int is_placeholder = 0;
    int i_out = 0;

    for (int i = 0; fmt[i] != '\0'; ++ i) {
        if (fmt[i] == '%')
            is_placeholder = 1;
        else if (is_placeholder == 1) {
            //there are only three cases: %02d %2d %d, so we need to note the length between % and d.
            //and only d u x have the %02
            char buffer[256];

            int count = 0;
            char modifiers[3];
            while (1) {
                if (count >= 3 || fmt[i] == '\0')
                    panic("Bad %... in *printf");
                if (count == 0 && fmt[i] == '0') {
                    modifiers[count] = '0';
                    count ++;
                }
                if (fmt[i] - '0' > 0 && fmt[i] - '0' < 10) {
                    modifiers[count] = fmt[i];
                    count ++;
                }

                if (fmt[i] == 'd' || fmt[i] == 'u' || fmt[i] == 'x') {
                    modifiers[count] = '\0';
                    number_replace(fmt[i], modifiers, buffer, &arg_list);
                    strcpy(out + i_out, buffer);
                    i_out += strlen(buffer);
                    break;
                }
                if (fmt[i] == 's') {
                    if (count >= 2)
                        panic("Bad %... in *printf");
                    modifiers[count] = '\0';
                    string_replace(modifiers, buffer, va_arg(arg_list, char *));
                    strcpy(out + i_out, buffer);
                    i_out += strlen(buffer);
                    break;
                }
                if (fmt[i] == 'c') {
                    if (count >= 2)
                        panic("Bad %... in *printf");
                    modifiers[count] = '\0';
                    char_replace(modifiers, buffer, va_arg(arg_list, int));
                    strcpy(out + i_out, buffer);
                    i_out += strlen(buffer);
                    break;
                }
                i++;
            }
            is_placeholder = 0;
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
