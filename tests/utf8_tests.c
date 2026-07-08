// You need to tell your compiler that the execution character set is UTF-8.
// MSVC cl: /utf-8

#include "../utf8.c"

void run_utf8_tests()
{
    test(utf8_char_len('C') == 1);
    test(utf8_str_len(fstr0(utf8"Brie de Meaux")) == 13);
    test(utf8_str_len(fstr0(utf8"中野")) == 2);

    u32 index = 0;
    utf8_str_for(fstr0(utf8"田端"), it, {
        if (!index) {
            // U+7530
            test(it.u8_1 == fstr0(utf8"田").data[0]);
            test(it.u8_2 == fstr0(utf8"田").data[1]);
            test(it.s8_3 == utf8"田"[2]);
        } else {
            test(it.s8_1 == utf8"端"[0]);
            test(it.s8_2 == utf8"端"[1]);
            test(it.s8_3 == utf8"端"[2]);
        }
        index++;
    });
    test(index == 2);
}
