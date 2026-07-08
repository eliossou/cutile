#ifndef CUT_MEMORY_DEBUG
    #define CUT_MEMORY_DEBUG

    #include "base.c"
    #include "memory.c"
    #ifdef CUT_MEMORY_DEBUG_STACKTRACE
        #include "stacktrace.c"
    #endif
    #include "hash_table.c"
    #include "dyn_array.c"

    typedef struct Cut_Mem_Allocation_Info {
        void *address;
        cut_uptrsize size;

        #ifndef CUT_MEMORY_DEBUG_STACKFRAMES
            #define CUT_MEMORY_DEBUG_STACKFRAMES 24
        #endif

        #ifdef CUT_MEMORY_DEBUG_STACKTRACE
            Cut_Stacktrace_Frame frames[CUT_MEMORY_DEBUG_STACKFRAMES];
            int frames_count;
        #endif
    } Cut_Mem_Allocation_Info;

    typedef struct Cut_Mem_Allocator_Debugger {
        Cut_Hash_Table allocated;
        Cut_Dyn_Array invalid_frees;
        cut_uptrsize total_allocated;
        cut_uptrsize total_freed;

        Cut_Mem_Allocator mem_allocator;
    } Cut_Mem_Allocator_Debugger;

    cut_inlinable Cut_Mem_Allocator_Debugger cut_mem_allocator_debugger_create(Cut_Mem_Allocator to_debug_allocator, Cut_Mem_Allocator mem_allocator)
    {
        Cut_Mem_Allocator_Debugger r = {
            .allocated = cut_hash_table_default(sizeof(void *), sizeof(Cut_Mem_Allocation_Info), mem_allocator),
            .invalid_frees = cut_dyn_array_create(sizeof(Cut_Mem_Allocation_Info), 256, 256, mem_allocator),
            .mem_allocator = to_debug_allocator
        };

        cut_hash_table_init(&r.allocated, 256);

        return r;
    }

    void *cut_mem_allocator_debugger_allocate(cut_uptrsize size, void *allocator_data);
    void cut_mem_allocator_debugger_free(void *ptr, void *allocator_data);

    Cut_Dyn_Array(cut_u8Arrview) cut_mem_allocator_debugger_report_generate(Cut_Mem_Allocator_Debugger *, Cut_Mem_Allocator);
    void                         cut_mem_allocator_debugger_report_destroy(Cut_Dyn_Array(cut_u8Arrview) *report);
#endif

#if !defined(CUT_MEMORY_DEBUG_IMPL_INCLUDED) && (defined(CUT_MEMORY_DEBUG_IMPL) || defined(CUT_IMPL))
    #define CUT_MEMORY_DEBUG_IMPL_INCLUDED

    #include "print.c"

    void *cut_mem_allocator_debugger_allocate(cut_uptrsize size, void *allocator_data)
    {
        Cut_Mem_Allocator_Debugger *debugger = (Cut_Mem_Allocator_Debugger *)allocator_data;

        void *address = cut_mem_allocate(size, &debugger->mem_allocator);

        Cut_Mem_Allocation_Info allocation = {
            .address = address,
            .size = size
        };
        debugger->total_allocated += size;

        #ifdef CUT_MEMORY_DEBUG_STACKTRACE
            cut_stacktrace(allocation.frames, &allocation.frames_count, array_size(allocation.frames), 2);
        #endif

        cut_hash_table_add(&debugger->allocated, &address, &allocation);

        return address;
    }

    void cut_mem_allocator_debugger_free(void *ptr, void *allocator_data)
    {
        Cut_Mem_Allocator_Debugger *debugger = (Cut_Mem_Allocator_Debugger *)allocator_data;
        Cut_Mem_Allocation_Info *allocation = cut_hash_table_get(&debugger->allocated, &ptr);

        if (allocation == 0) {
            Cut_Mem_Allocation_Info invalid_free = {
                .address = ptr
            };

            #ifdef CUT_MEMORY_DEBUG_STACKTRACE
                cut_stacktrace(invalid_free.frames, &invalid_free.frames_count, array_size(invalid_free.frames), 2);
            #endif
            cut_dyn_array_add_back(&debugger->invalid_frees, &invalid_free);

            return;
        } else {
            cut_hash_table_remove(&debugger->allocated, &ptr);
            debugger->total_freed += allocation->size;
        }

        cut_mem_free(ptr, &debugger->mem_allocator);
    }

    static inline void cut_mem_allocator_debugger_report_generate_stacktrace(Cut_Mem_Allocator mem_allocator, Cut_Dyn_Array *report, Cut_Mem_Allocation_Info *alloc_info)
    {
        #ifdef CUT_MEMORY_DEBUG_STACKTRACE
        {
            cut_u8Arrview str;
            for (int i = 0; i < alloc_info->frames_count; i++) {
                Cut_Stacktrace_Frame *fr = alloc_info->frames + i;
                cut_u8Arrview routine_name = cut_u8arrview_ptr(fr->routine_name, fr->routine_name_length);
                cut_u8Arrview filename = cut_u8arrview_ptr(fr->filename, fr->filename_length);
                str = cut_sprint(
                    &mem_allocator,
                    cut_fstr0("    at %(%) (%:%).\n"),
                    cut_format_address(fr->program_counter),
                    cut_format_str(routine_name),
                    cut_format_str(filename),
                    cut_format_int(fr->line)
                );
                cut_dyn_array_add_back(report, &str);
            }
        }
        #endif
    }

    Cut_Dyn_Array(cut_u8Arrview) cut_mem_allocator_debugger_report_generate(Cut_Mem_Allocator_Debugger *debugger, Cut_Mem_Allocator mem_allocator)
    {
        Cut_Dyn_Array report = cut_dyn_array(sizeof(cut_u8Arrview), debugger->allocated.count, mem_allocator);
        cut_u8Arrview str;

        cut_hash_table_for(&debugger->allocated, void *key, Cut_Mem_Allocation_Info *alloc_info, {
            str = cut_sprint(
                &mem_allocator,
                cut_fstr0("Leaked % bytes at address %.\n"),
                cut_format_int(alloc_info->size),
                cut_format_address(alloc_info->address)
            );
            cut_dyn_array_add_back(&report, &str);

            cut_mem_allocator_debugger_report_generate_stacktrace(mem_allocator, &report, alloc_info);
        });

        cut_dyn_array_for(&debugger->invalid_frees, Cut_Mem_Allocation_Info *alloc_info, {
            str = cut_sprint(
                &mem_allocator,
                cut_fstr0("Invalid free of address %.\n"),
                cut_format_address(alloc_info->address)
            );
            cut_dyn_array_add_back(&report, &str);

            cut_mem_allocator_debugger_report_generate_stacktrace(mem_allocator, &report, alloc_info);
        });

        str = cut_sprint(
            &mem_allocator,
            cut_fstr0("Total allocated % bytes.\n"),
            cut_format_int(debugger->total_allocated)
        );
        cut_dyn_array_add_back(&report, &str);

        str = cut_sprint(
            &mem_allocator,
            cut_fstr0("Total freed: % bytes.\n"),
            cut_format_int(debugger->total_freed)
        );
        cut_dyn_array_add_back(&report, &str);

        return report;
    }

    void cut_mem_allocator_debugger_report_destroy(Cut_Dyn_Array(cut_u8Arrview) *report)
    {
        cut_dyn_array_for(report, cut_u8Arrview *it, {
            cut_mem_free(it->data, &report->mem_allocator);
        });
        cut_dyn_array_deinit(report);
    }
#endif
