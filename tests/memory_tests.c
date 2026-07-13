#include "../memory.c"

void run_memory_tests()
{
    u8 buf1[] = {1, 2, 3, 4, 5};
    u8 buf2[] = {1, 2, 3, 4, 5};
    u8 buf3[] = {1, 2, 3, 0, 5};

    test(!mem_cmp(buf1, buf2, 5));
    test(mem_cmp(buf1, buf3, 5));
    test(!mem_cmp(buf1, buf3, 3));

    test(mem_is_same(buf1, buf2, 5));
    test(!mem_is_same(buf1, buf3, 5));

    u8 dest[5] = {0};
    mem_cpy(dest, buf1, 5);
    test(mem_is_same(dest, buf1, 5));

    mem_set(dest, 0xFF, 5);
    test(dest[0] == 0xFF);
    test(dest[4] == 0xFF);
    test(dest[2] == 0xFF);

    // mem_cmp_fast
    {
        u8 a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        u8 b[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        u8 c[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 0};

        test(!mem_cmp_fast(a, b, 10));
        test(mem_cmp_fast(a, c, 10));

        // Exactly 8 bytes boundary.
        test(!mem_cmp_fast(a, b, 8));
        test(!mem_cmp_fast(a, c, 8));
        test(mem_cmp_fast(a+2, c+2, 8));

        // Less than 8 bytes.
        test(!mem_cmp_fast(a, b, 3));
        test(!mem_cmp_fast(a, c, 3));
        test(mem_cmp_fast(a+7, c+7, 3));

        // Empty range.
        test(!mem_cmp_fast(a, b, 0));
    }

    // mem_cpy_fast
    {
        u8 src[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

        // Copy more than 8 bytes.
        u8 dst1[12] = {0};
        mem_cpy_fast(dst1, src, 12);
        test(mem_is_same(dst1, src, 12));

        // Copy exactly 8 bytes.
        u8 dst2[8] = {0};
        mem_cpy_fast(dst2, src, 8);
        test(mem_is_same(dst2, src, 8));

        // Copy less than 8 bytes.
        u8 dst3[3] = {0};
        mem_cpy_fast(dst3, src, 3);
        test(mem_is_same(dst3, src, 3));

        // Copy 0 bytes.
        u8 dst4[4] = {0};
        mem_cpy_fast(dst4, src, 0);
        test(dst4[0] == 0);
    }
}
