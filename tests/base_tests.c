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

    // u8arrview_eq
    cut_u8 a1[] = {1, 2, 3};
    cut_u8 a2[] = {1, 2, 3};
    cut_u8 a3[] = {1, 2, 4};
    Cut_u8Arrview v1 = cut_u8arrview(a1);
    Cut_u8Arrview v2 = cut_u8arrview(a2);
    Cut_u8Arrview v3 = cut_u8arrview(a3);
    Cut_u8Arrview v4 = cut_u8arrview_ptr(a1, 2);
    test(cut_u8arrview_eq(&v1, &v2));
    test(!cut_u8arrview_eq(&v1, &v3));
    test(!cut_u8arrview_eq(&v1, &v4));

    // u8arrview_0p
    cut_u8Arrview v0 = cut_u8arrview_0p("hello");
    test(v0.count == 5);
    test(v0.data[0] == 'h');

    // u8arrview_0
    cut_u8 hello[] = "hello";
    cut_u8Arrview v0b = cut_u8arrview_0(hello);
    test(v0b.count == 5);

    // fstr0
    cut_u8Arrview vf = cut_fstr0("abc");
    test(vf.count == 3);
    test(vf.data[0] == 'a');
}
