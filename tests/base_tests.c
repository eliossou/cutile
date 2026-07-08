#include "../base.c"

cut_thread_local int dummy = 2;

void run_base_tests()
{
    test(sizeof(s8) == 1);
    test(sizeof(u8) == 1);
    test(sizeof(s16) == 2);
    test(sizeof(u16) == 2);
    test(sizeof(s32) == 4);
    test(sizeof(u32) == 4);
    test(sizeof(s64) == 8);
    test(sizeof(u64) == 8);

    test(sizeof(f32) == 4);
    test(sizeof(f64) == 8);

    packed(
        struct dummy {
            int a;
            int b;
            char c;
        }
    ) dummy1;
    test(cut_field_offset(struct dummy, a) == 0);
    test(cut_field_offset(struct dummy, b) == 4);
    test(cut_field_offset(struct dummy, c) == 8);

    int arr[50];
    test(cut_array_size(arr) == 50);

    test(cut_align_nb(2, 2) == 2);
    test(cut_align_nb(2, 4) == 4);
    test(cut_align_nb(16, 8) == 16);
    test(cut_align_nb(17, 8) == 24);
    test(cut_align_nb(17, 32) == 32);
}
