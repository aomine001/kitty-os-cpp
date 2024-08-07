//
// Created by Piotr on 03.05.2024.
//

#ifndef KITTY_OS_CPP_KSTDIO_HPP
#define KITTY_OS_CPP_KSTDIO_HPP

#include <external/flanterm/backends/fb.h>
#include <external/flanterm/flanterm.h>
#include <ccmath/basic.hpp>
#include <hal/x64/io.hpp>
#include <sys/types.h>
#include <stdint.h>

namespace kstd
{
    extern flanterm_context* ft_ctx;

    void reinit_term();

    void InitializeTerminal();
    void puts(const char* s);
    void putc(const char c);

    void move_cursor_x(int off);
    void move_cursor_y(int off);
    void clear_x_cursor();
    void clear_y_cursor();
    int get_cursor_x();
    int get_cursor_y();

    void enable_tailing_zeroes();
    void disable_tailing_zeroes();

    void print_signed_integer(signed int si);
    void print_unsigned_integer_octal(unsigned int si);
    void print_unsigned_integer_hexadecimal(unsigned int si);
    void print_unsigned_integer(unsigned int ui);
    void print_double(double d);
    void print_pointer(void* ptr);
    void print_long(long l);
    void print_unsigned_long_octal(unsigned long ul);
    void print_unsigned_long_hexadecimal(unsigned long ul);
    void print_unsigned_long_integer(unsigned long ul);
    void print_long_long_integer(long long ll);
    void print_unsigned_long_long_hexadecimal(unsigned long long ull);
    void print_unsigned_char_hexadecimal(unsigned char uc);
    void print_unsigned_char_integer(unsigned char uc);
    void print_unsigned_short_hexadecimal(unsigned short us);
    void print_unsigned_short_integer(unsigned short usi);
    void print_signed_char_integer(signed char sci);
    void print_signed_short_integer(signed short ssi);

    void printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
}

#endif //KITTY_OS_CPP_KSTDIO_HPP
