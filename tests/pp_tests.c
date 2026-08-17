#include "../pp.c"

#define count_fwd(...) cut_va_args_count(__VA_ARGS__)

#define strfy(x) #x
#define add(x) + (x)
#define comma(x) x,
#define cut_enum_field(x) cut_pp_e_##x,

enum { cut_va_args_for_each(cut_enum_field, red, green, blue) cut_pp_e_count };

void run_pp_tests()
{
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

    // For each
    test(cut_pp_e_count == 3);
    test(cut_pp_e_red == 0);
    test(cut_pp_e_green == 1);
    test(cut_pp_e_blue == 2);

    test(str0_eq(cut_va_args_for_each(strfy, a, b, c), "abc"));
    test(str0_eq(cut_va_args_for_each(strfy, 0, 1, 2, 3), "0123"));
    test(str0_eq(cut_va_args_for_each(strfy, a, b + c, (x)(y), "str"), "ab + c(x)(y)\"str\""));

    // For each is usable wherever a list of tokens is expected.
    int vals[] = { cut_va_args_for_each(comma, 10, 20, 30, 40) };
    test(cut_array_size(vals) == 4);
    test(vals[0] == 10 && vals[1] == 20 && vals[2] == 30 && vals[3] == 40);

    int total = (0 cut_va_args_for_each(add, 1, 2, 3));
    test(total == 6);

    int total16 = (0 cut_va_args_for_each(add, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
    test(total16 == 136);
}