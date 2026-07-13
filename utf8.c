#ifndef CUT_UTF8
    #define CUT_UTF8

    #include "base.c"

    // This macro makes sure UTF8 is used as encoding for string literals.
    // Use this as a string literal prefix. Example: cut_utf8"Foo".
    // The type it evaluates to is char[N] where N is the size of the string in UTF-8 code units including the null terminator.
    #define cut_utf8

    // The following functions assume the values passed are valid UTF-8 input!
    cut_inlinable int     cut_utf8_char_len(cut_u8 c); // Returns the length of an UTF8 character, [1, 4].
    cut_inlinable cut_u32 cut_utf8_char_codepoint(cut_u8 c[]); // Returns the Unicode codepoint of a given UTF8 character.
    cut_inlinable int     cut_utf8_str_len(Cut_u8Arrview str); // Returns the number of UTF8 characters inside an UTF8 string.

    typedef union Cut_Utf8_Str_Iterator {
        struct {
            cut_u8 u8_[4];
        };
        struct {
            cut_u8 u8_1;
            cut_u8 u8_2;
            cut_u8 u8_3;
            cut_u8 u8_4;
        };
        struct {
            cut_s8 s8_[4];
        };
        struct {
            cut_s8 s8_1;
            cut_s8 s8_2;
            cut_s8 s8_3;
            cut_s8 s8_4;
        };
        cut_u32 u32_;
        cut_s32 s32_;
    } Cut_Utf8_Str_Iterator;

    // UTF-8 string iteration.
    // u8arrviewStr: the string to iterate, represented in a cut_u8Arrview.
    // Utf8StrItName: the name of the iterator you can read in the given code.
    // Code: your code.
    #define cut_utf8_str_for(u8ArrviewStr, Utf8StrItName, Code)

    #ifdef CUT_SHORT_NAMES
        #define utf8 cut_utf8
        #define utf8_char_len       cut_utf8_char_len
        #define utf8_char_codepoint cut_utf8_char_codepoint
        #define utf8_str_len        cut_utf8_str_len
        #define Utf8_Str_Iterator   Cut_Utf8_Str_Iterator
        #define utf8_str_for        cut_utf8_str_for
    #endif
#endif

#ifndef CUT_UTF8_INL_IMPL
    #define CUT_UTF8_INL_IMPL

    /* Here are some rules from the RFC regarding bytes sequences in an UTF-8 string:

        https://www.rfc-editor.org/rfc/rfc3629#section-4

        Copyright Notice

        Copyright (C) The Internet Society (2003).  All Rights Reserved.

        4.  Syntax of UTF-8 Byte Sequences

        For the convenience of implementors using ABNF, a definition of UTF-8
        in ABNF syntax is given here.

        A UTF-8 string is a sequence of octets representing a sequence of UCS
        characters.  An octet sequence is valid UTF-8 only if it matches the
        following syntax, which is derived from the rules for encoding UTF-8
        and is expressed in the ABNF of [RFC2234].

        UTF8-octets = *( UTF8-char )
        UTF8-char   = UTF8-1 / UTF8-2 / UTF8-3 / UTF8-4
        UTF8-1      = %x00-7F
        UTF8-2      = %xC2-DF UTF8-tail

        UTF8-3      = %xE0 %xA0-BF UTF8-tail / %xE1-EC 2( UTF8-tail ) /
                      %xED %x80-9F UTF8-tail / %xEE-EF 2( UTF8-tail )
        UTF8-4      = %xF0 %x90-BF 2( UTF8-tail ) / %xF1-F3 3( UTF8-tail ) /
                      %xF4 %x80-8F 2( UTF8-tail )
        UTF8-tail   = %x80-BF
    */

    cut_inlinable int cut_utf8_char_len(cut_u8 c)
    {
        if (c <= 0x7F)
            return 1;
        if (c <= 0xDF)
            return 2;
        if (c <= 0xEF)
            return 3;
        return 4;
    }

    cut_inlinable cut_u32 cut_utf8_char_codepoint(cut_u8 c[])
    {
        if (c[0] <= 0x7F)
            return c[0];
        if (c[0] <= 0xDF)
            return (((cut_s32)c[0] & 0x1F) << 6) | (c[1] & 0x3F);
        if (c[0] <= 0xEF)
            return (((cut_s32)c[0] & 0x0F) << 12) | (((cut_s32)c[1] & 0x3F) << 6) | (c[2] & 0x3F);
        return (((cut_s32)c[0] & 0x07) << 18) | (((cut_s32)c[1] & 0x3F) << 12) | (((cut_s32)c[2] & 0x3F) << 6) | (c[3] & 0x3F);
    }

    cut_inlinable int cut_utf8_str_len(cut_u8Arrview str)
    {
        cut_u32 count = 0;
        cut_u32 i = 0;
        while (i < str.count) {
            i += cut_utf8_char_len(str.data[i]);
            count += 1;
        }
        return count;
    }

    #undef cut_utf8
    #define cut__utf8_glue(a, b, c) a##b##c     // In case u8 is already a macro, make sure it does not expand.
    #define cut_utf8 cut__utf8_glue(u, 8, "")   // utf8("MY STRING LITERAL") expands to u8"" "MY STRING LITERAL"
                                                // This is ok. According to ISO C: If one literal is unprefixed, the resulting string literal has the width/encoding specified by the prefixed literal.

    #undef cut_utf8_str_for
    #define cut_utf8_str_for(u8ArrviewStr, Utf8StrItName, Code)                     \
    {                                                                               \
        cut_u32 cut__utf8_str_for_i = 0;                                            \
        Cut_Utf8_Str_Iterator Utf8StrItName;                                        \
        while (cut__utf8_str_for_i < (u8ArrviewStr).count) {                        \
            if ((u8ArrviewStr).data[cut__utf8_str_for_i] <= 0x7F) {                 \
                Utf8StrItName.u8_1 = (u8ArrviewStr).data[cut__utf8_str_for_i];      \
                Utf8StrItName.u8_2 = 0;                                             \
                Utf8StrItName.u8_3 = 0;                                             \
                Utf8StrItName.u8_4 = 0;                                             \
                cut__utf8_str_for_i += 1;                                           \
            } else if ((u8ArrviewStr).data[cut__utf8_str_for_i] <= 0xDF) {          \
                Utf8StrItName.u8_1 = (u8ArrviewStr).data[cut__utf8_str_for_i];      \
                Utf8StrItName.u8_2 = (u8ArrviewStr).data[cut__utf8_str_for_i+1];    \
                Utf8StrItName.u8_3 = 0;                                             \
                Utf8StrItName.u8_4 = 0;                                             \
                cut__utf8_str_for_i += 2;                                           \
            } else if ((u8ArrviewStr).data[cut__utf8_str_for_i] <= 0xEF) {          \
                Utf8StrItName.u8_1 = (u8ArrviewStr).data[cut__utf8_str_for_i];      \
                Utf8StrItName.u8_2 = (u8ArrviewStr).data[cut__utf8_str_for_i+1];    \
                Utf8StrItName.u8_3 = (u8ArrviewStr).data[cut__utf8_str_for_i+2];    \
                Utf8StrItName.u8_4 = 0;                                             \
                cut__utf8_str_for_i += 3;                                           \
            } else {                                                                \
                Utf8StrItName.u8_1 = (u8ArrviewStr).data[cut__utf8_str_for_i];      \
                Utf8StrItName.u8_2 = (u8ArrviewStr).data[cut__utf8_str_for_i+1];    \
                Utf8StrItName.u8_3 = (u8ArrviewStr).data[cut__utf8_str_for_i+2];    \
                Utf8StrItName.u8_4 = (u8ArrviewStr).data[cut__utf8_str_for_i+3];    \
                cut__utf8_str_for_i += 4;                                           \
            }                                                                       \
                                                                                    \
            Code;                                                                   \
        }                                                                           \
    }
#endif
