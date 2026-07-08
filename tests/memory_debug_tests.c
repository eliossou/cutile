#define CUT_MEMORY_DEBUG_STACKTRACE
#include "../memory_debug.c"

void run_memory_debug_tests()
{
    Cut_Mem_Allocator_Debugger mem_allocator_debugger = cut_mem_allocator_debugger_create(memallocator, memallocator);

    Cut_Mem_Allocator temp_mem_allocator = {
        .data = &mem_allocator_debugger,
        .allocate = cut_mem_allocator_debugger_allocate,
        .free = cut_mem_allocator_debugger_free
    };

    void *mem560 = cut_mem_allocate(560, &temp_mem_allocator);
    test(mem_allocator_debugger.total_allocated == 560);

    cut_mem_free(mem560, &temp_mem_allocator);
    test(mem_allocator_debugger.total_allocated == 560);
    test(mem_allocator_debugger.total_freed == 560);

    cut_mem_free((void*)0x480, &temp_mem_allocator);    // Invalid free.
    cut_mem_allocate(10, &temp_mem_allocator);
    cut_mem_allocate(42, &temp_mem_allocator);
    test(mem_allocator_debugger.total_allocated == 560+10+42);
    test(mem_allocator_debugger.total_freed == 560);

    // Report: 2 leaks, one invalid free.
    Dyn_Array(u8arrview) report = cut_mem_allocator_debugger_report_generate(&mem_allocator_debugger, memallocator);
    // 3 for bad allocatior operations + 2 for total allocated/total freed + unknown number for the stacktrace. So at least 5.
    test(report.count >= 5);

    cut_dyn_array_for(&report, cut_u8Arrview *it, {
        cut_print(*it);
    });

    cut_mem_allocator_debugger_report_destroy(&report);
}
