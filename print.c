#ifndef CUT_PRINT
    #define CUT_PRINT

    #include <stdarg.h>
    #include "base.c"
    #include "memory.c"

    cut_u8Arrview cut_sprint_v(Cut_Mem_Allocator *, cut_u8Arrview format, va_list args);
    cut_u8Arrview cut_sprint(Cut_Mem_Allocator *, cut_u8Arrview format, ...);

    #ifndef CUT_PRINT_FLUSH_SIZE
        #define CUT_PRINT_FLUSH_SIZE 128
    #endif

    void cut_print_v(cut_u8Arrview format, va_list args);
    void cut_print(cut_u8Arrview format, ...);

    #define cut_print2(Array0Format, ...) cut_print(cut_fstr0(Array0Format), ## __VA_ARGS__)

    #define cut_format_int(Integer) &(Cut_Format_Int){ cut_format_int_count, cut_format_int_proc, Integer }
    #define cut_format_int_hex(Integer) &(Cut_Format_Int_Hex){ cut_format_int_hex_count, cut_format_int_hex_proc, Integer }
    #define cut_format_address(Ptr) cut_format_int_hex((cut_uptrsize)Ptr)
    #define cut_format_str(StrView) &(Cut_Format_Str){ cut_format_str_count, cut_format_str_proc, StrView }
    #define cut_format_str0(Str0) &(Cut_Format_Str0){ cut_format_str0_count, cut_format_str0_proc, Str0 }

    typedef struct Cut_Formatter {
        int (*count)(void *param);
        void (*proc)(void *param, cut_u8 *data, cut_u32 *index);
    } Cut_Formatter;

    typedef struct Cut_Format_Int {
        Cut_Formatter base;
        cut_s64 value;
    } Cut_Format_Int;
    int cut_format_int_count(void *param);
    void cut_format_int_proc(void *param, cut_u8 *data, cut_u32 *index);

    typedef struct Cut_Format_Int_Hex {
        Cut_Formatter formatter;

        cut_s64 value;
    } Cut_Format_Int_Hex;
    int cut_format_int_hex_count(void *param);
    void cut_format_int_hex_proc(void *param, cut_u8 *data, cut_u32 *index);

    int cut_format_str_count(void *param);
    void cut_format_str_proc(void *param, cut_u8 *data, cut_u32 *index);
    typedef struct Cut_Format_Str {
        Cut_Formatter formatter;

        cut_u8Arrview str;
    } Cut_Format_Str;

    int cut_format_str0_count(void *param);
    void cut_format_str0_proc(void *param, cut_u8 *data, cut_u32 *index);
    typedef struct {
        Cut_Formatter formatter;

        void *str0;
    } Cut_Format_Str0;

    #if defined(CUT_PRINT_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        #define sprint_v(...) cut_sprint_v(__VA_ARGS__);
        #define sprint(...)   cut_sprint(__VA_ARGS__);

        #define print_v(...) cut_print_v(__VA_ARGS__)
        #define print(...)   cut_print(__VA_ARGS__)
        #define print2       cut_print2

        typedef Cut_Formatter Formatter;

        #define format_int     cut_format_int
        #define format_int_hex cut_format_int_hex
        #define format_address cut_format_address
        #define format_str     cut_format_str
        #define format_str0    cut_format_str0
    #endif
#endif

#if !defined(CUT_PRINT_IMPL_INCLUDED) && (defined(CUT_PRINT_IMPL) || defined(CUT_IMPL))
    #define CUT_PRINT_IMPL_INCLUDED

    #if CUT_TARGET_OS == CUT_WINDOWS
        #include "win32.c"
    #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        #include <unistd.h>
    #endif

    cut_u8Arrview cut_sprint_v(Cut_Mem_Allocator *mem_allocator, cut_u8Arrview format, va_list args)
    {
        va_list args2;
        va_copy(args2, args);

        cut_u32 count = 0;

        Cut_Formatter *param;

        for (cut_u32 i = 0; i < format.count; i++) {
            if (format.data[i] == '%') {
                param = va_arg(args, Cut_Formatter *);
                count += param->count(param);
            } else {
                count++;
            }
        }

        cut_u8Arrview result = {
            .data = cut_mem_allocate(count * sizeof(cut_u8), mem_allocator),
            .count = count
        };

        cut_u32 index = 0;
        for (cut_u32 i = 0; i < format.count; i++) {
            if (format.data[i] == '%') {
                param = va_arg(args2, Cut_Formatter *);
                param->proc(param, result.data, &index);
            } else {
                result.data[index] = format.data[i];
                index++;
            }
        }

        va_end(args2);

        return result;
    }

    cut_u8Arrview cut_sprint(Cut_Mem_Allocator *mem_allocator, cut_u8Arrview format, ...)
    {
        va_list args;
        va_start(args, format);

        cut_u8Arrview result = cut_sprint_v(mem_allocator, format, args);

        va_end(args);

        return result;
    }

    void cut_print_v(cut_u8Arrview format, va_list args)
    {
        persist cut_u8 buf[CUT_PRINT_FLUSH_SIZE];
        cut_u32 i = 0;
        cut_u32 count = 0;
        cut_u32 tmp_count = 0;
        Cut_Formatter *param;

        #if CUT_TARGET_OS == CUT_WINDOWS
            cut_WIN32_HANDLE out = cut_WIN32_GetStdHandle(cut_WIN32_STD_OUTPUT_HANDLE);
            cut_WIN32_DWORD written;
        #endif

        while (i < format.count) {
            if (format.data[i] == '%') {
                param = va_arg(args, Cut_Formatter *);
                tmp_count = param->count(param);
                if (tmp_count + count > CUT_PRINT_FLUSH_SIZE) {
                    #if CUT_TARGET_OS == CUT_WINDOWS
                        if (out && out != cut_WIN32_INVALID_HANDLE_VALUE)
                            WriteConsoleA(out, buf, count, &written, 0);
                    #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
                        write(1, buf, count);
                    #endif

                    count = 0;
                } else {
                    param->proc(param, buf, &count);
                }
            } else {
                buf[count] = format.data[i];
                count++;
            }

            i++;
        }

        if (count) {
            #if CUT_TARGET_OS == CUT_WINDOWS
                if (out && out != cut_WIN32_INVALID_HANDLE_VALUE)
                    WriteConsoleA(out, buf, count, &written, 0);
            #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
                write(1, buf, count);
            #endif
        }

        return;
    }

    void cut_print(cut_u8Arrview format, ...)
    {
        va_list args;
        va_start(args, format);

        cut_print_v(format, args);

        va_end(args);
    }

    cut_inlinable int cut_format_int_digits_count(cut_s64 nb)
    {
        cut_u64 v = nb < 0 ? -(cut_u64)nb : (cut_u64)nb;

        if (v < 10)
            return 1;
        else if (v < 100)
            return 2;
        else if (v < 1000)
            return 3;
        else if (v < 10000)
            return 4;
        else if (v < 100000)
            return 5;
        else if (v < 1000000)
            return 6;
        else if (v < 10000000)
            return 7;
        else if (v < 100000000)
            return 8;
        else if (v < 1000000000)
            return 9;
        else if (v < 10000000000)
            return 10;
        else if (v < 100000000000)
            return 11;
        else if (v < 1000000000000)
            return 12;
        else if (v < 10000000000000)
            return 13;
        else if (v < 100000000000000)
            return 14;
        else if (v < 1000000000000000)
            return 15;
        else if (v < 10000000000000000)
            return 16;
        else if (v < 100000000000000000)
            return 17;
        else if (v < 1000000000000000000)
            return 18;
        else
            return 19;
    }

    int cut_format_int_count(void *param)
    {
        Cut_Format_Int *format = (Cut_Format_Int *)param;

        unsigned negative = ((cut_u64)format->value) >> 63;

        return cut_format_int_digits_count(format->value) + negative;
    }

    void cut_format_int_proc(void *param, cut_u8 *data, cut_u32 *index)
    {
        Cut_Format_Int *format = (Cut_Format_Int *)param;
        unsigned negative = ((cut_u64)format->value) >> 63;

        data[*index] = '-';
        *index += negative;

        // @TODO: Do we really need the number of digits?
        int nb_digits = cut_format_int_digits_count(format->value);

        cut_s8 remainder;
        for (int i = nb_digits - 1; i >= 0; i--) {
            remainder = format->value % 10;
            remainder = remainder < 0 ? -remainder : remainder;
            data[(*index) + i] = remainder + '0';
            format->value /= 10;
        }
        *index += nb_digits;
    }

    cut_inlinable int cut_format_int_hex_digits_count(cut_s64 nb)
    {
        cut_u64 v = nb < 0 ? -(cut_u64)nb : (cut_u64)nb;

        if (v <= 0xF)
            return 1;
        else if (v <= 0xFF)
            return 2;
        else if (v <= 0xFFF)
            return 3;
        else if (v <= 0xFFFF)
            return 4;
        else if (v <= 0xFFFFF)
            return 5;
        else if (v <= 0xFFFFFF)
            return 6;
        else if (v <= 0xFFFFFFF)
            return 7;
        else if (v <= 0xFFFFFFFF)
            return 8;
        else if (v <= 0xFFFFFFFFF)
            return 9;
        else if (v <= 0xFFFFFFFFFF)
            return 10;
        else if (v <= 0xFFFFFFFFFFF)
            return 11;
        else if (v <= 0xFFFFFFFFFFFF)
            return 12;
        else if (v <= 0xFFFFFFFFFFFFF)
            return 13;
        else if (v <= 0xFFFFFFFFFFFFFF)
            return 14;
        else if (v <= 0xFFFFFFFFFFFFFFF)
            return 15;
        else
            return 16;
    }

    int cut_format_int_hex_count(void *param)
    {
        Cut_Format_Int_Hex *format = (Cut_Format_Int_Hex *)param;

        return cut_format_int_hex_digits_count(format->value) + 2;
    }

    void cut_format_int_hex_proc(void *param, cut_u8 *data, cut_u32 *index)
    {
        Cut_Format_Int_Hex *format = (Cut_Format_Int_Hex *)param;

        data[(*index)] = '0';
        data[(*index) + 1] = 'x';
        (*index) += 2;

        cut_persist cut_u8 tbl[] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F'
        };

        // @TODO: Do we really need the number of digits here?
        int nb_digits = cut_format_int_hex_digits_count(format->value);

        cut_s8 tblindex;
        for (int i = nb_digits - 1; i >= 0; i--) {
            tblindex = format->value % 16;
            tblindex = tblindex < 0 ? -tblindex : tblindex;
            data[(*index) + i] = tbl[tblindex];
            format->value /= 16;
        }
        (*index) += nb_digits;
    }

    int cut_format_str_count(void *param)
    {
        Cut_Format_Str *format = (Cut_Format_Str *)param;

        return format->str.count;
    }

    void cut_format_str_proc(void *param, cut_u8 *data, cut_u32 *index)
    {
        Cut_Format_Str *format = (Cut_Format_Str *)param;

        cut_mem_cpy(data + *index, format->str.data, format->str.count);

        *index += format->str.count;
    }

    int cut_format_str0_count(void *param)
    {
        Cut_Format_Str0 *format = (Cut_Format_Str0 *)param;

        cut_u32 count = 0;
        while (((cut_u8 *)format->str0)[count])
            count++;

        return count;
    }

    void cut_format_str0_proc(void *param, cut_u8 *data, cut_u32 *index)
    {
        Cut_Format_Str0 *format = (Cut_Format_Str0 *)param;

        cut_u8 *str0 = format->str0;
        while (*str0) {
            data[*index] = *str0;
            (*index)++;
            str0++;
        }
    }
#endif
