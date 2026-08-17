#include "../pp.c"

#define count_fwd(...) cut_va_args_count(__VA_ARGS__)

enum { cut_pp_test_n3 = cut_va_args_count(1, 2, 3) };

void run_pp_tests()
{
    test(cut_pp_test_n3 == 3);

    test(cut_va_args_count(0) == 1);
    test(count_fwd(0) == 1);

    test(cut_va_args_count(0, 1) == 2);
    test(count_fwd(0, 1) == 2);

    test(cut_va_args_count(0, 1, 2) == 3);

    test(cut_va_args_count(0, 1, 2, 3, 4) == 5);
    test(cut_va_args_count(0, 1, 2, 3, 4, 5, 6, 7) == 8);

    test(cut_va_args_count(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15) == 16);

    // Works inside other variadic macros.
    test(count_fwd(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) == 13);

    // Arguments can be any token sequences, not just literals.
    test(cut_va_args_count(a, b + c, (x)(y), "str") == 4);

    // Result is usable wherever an integer constant expression is expected.
    int arr[cut_va_args_count(a, b, c, d)];
    test(cut_array_size(arr) == 4);
}