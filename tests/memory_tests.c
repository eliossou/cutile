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
}
