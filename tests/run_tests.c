#define CUT_IMPL
#define CUT_SHORT_NAMES
#include "../base.c"
#include "../print.c"
#include "../str.c"
#include "../memory.c"

Mem_Allocator memallocator;

int succeded_tests = 0;
int failed_tests = 0;

#define ppstr2(v) #v
#define ppstr(v) ppstr2(v)

#define test2(exp, exps) {                                                       \
    if (!(exp)) {                                                                \
        failed_tests++;                                                          \
        print(fstr0("FAILED: " exps " (" __FILE__ "." ppstr(__LINE__) ").\n"));  \
    } else {                                                                     \
        succeded_tests++;                                                        \
    }                                                                            \
}

#define test(exp) test2(exp, #exp)

#include "base_tests.c"
#include "dyn_array_tests.c"
#include "file_tests.c"
#include "hash_table_tests.c"
#include "ini_tests.c"
#include "memory_tests.c"
#include "memory_debug_tests.c"
#include "print_tests.c"
#include "stacktrace_tests.c"
#include "str_tests.c"
#include "utf8_tests.c"

void free_nothing(void *unused1, void *unused2)
{
}

void *resize_nothing(uptrsize size, void *old, uptrsize old_size, void *unused)
{
    void *new = mem_allocate(size, &memallocator);
    for (uptrsize i = 0; i < MIN(size, old_size); i++) {
        ((u8 *)new)[i] = ((u8 *)old)[i];
    }
    return new;
}

int main()
{
    Virt_Mem virt = {
        .reserved_size = 64 * 1024 * 1024, // 64 MB
        .commit_size = 4096,
        .alignment = 8
    };

    memallocator.data = &virt;
    memallocator.allocate = cut_virt_mem_allocate;
    memallocator.free = free_nothing;
    memallocator.resize = resize_nothing;

    virt_mem_init(&virt);

    print(fstr0("Running tests...\n"));

    test(stacktrace_api_init());

    run_base_tests();
    run_dyn_array_tests();
    run_file_tests();
    run_hash_table_tests();
    run_ini_tests();
    run_memory_tests();
    run_memory_debug_tests();
    run_print_tests();
    run_stacktrace_tests();
    run_str_tests();
    run_utf8_tests();

    print(
        fstr0("Tests finished: % succeeded, % failed.\n"),
        format_int(succeded_tests),
        format_int(failed_tests)
    );

    return failed_tests ? 1 : 0;
}
