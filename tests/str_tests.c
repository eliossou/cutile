#include "../str.c"

void run_str_tests()
{
    test(cut_str0_len("") == 0);
    test(cut_str0_len("hello") == 5);
    test(cut_str0_len("abc\0def") == 3);

    test(cut_str0_eq("abc", "abc"));
    test(!cut_str0_eq("abc", "abd"));
    test(!cut_str0_eq("abc", "ab"));

    Cut_u8Arrview v = cut_str0v("hello");
    test(v.count == 5);
    test(v.data[0] == 'h');
    test(v.data[4] == 'o');
}
