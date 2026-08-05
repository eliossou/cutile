#ifndef CUT_MEMORY
    #define CUT_MEMORY

    #include <string.h>
    #include "base.c"

    #define cut_arrview_is_same(ArrViewPtr1, ArrViewPtr2)   \
        ((ArrViewPtr1)->count == (ArrViewPtr2)->count && !memcmp((ArrViewPtr1)->data, (ArrViewPtr2)->data, (ArrViewPtr2)->count))

    typedef struct Cut_Buffer_Arena {
        cut_u8 *buffer;
        cut_uptrsize size;
        cut_uptrsize allocated;
    } Cut_Buffer_Arena;

    void *cut_buffer_arena_allocate(cut_uptrsize size, Cut_Buffer_Arena *buffer_mem);
    void cut_buffer_arena_reset(Cut_Buffer_Arena *buffer_mem);

    typedef struct Cut_Arena {
        cut_uptrsize reserved_size;
        cut_uptrsize commit_size;
        // Must be a power of 2.
        cut_u32 alignment;

        cut_u8 *start;
        cut_u8 *current;
        cut_u8 *uncommitted_start;
        cut_u8 *end;
    } Cut_Arena;

    cut_inlinable Cut_Arena cut_arena_create(cut_uptrsize size_to_reserve, cut_uptrsize commit_size, cut_u32 alignment);
    #define                 cut_arena_create_(size_to_reserve, commit_size) cut_arena_create(size_to_reserve, commit_size, 8)

    int   cut_arena_init(Cut_Arena *);
    void  cut_arena_destroy(Cut_Arena *);
    void *cut_arena_allocate(cut_uptrsize size, Cut_Arena *);
    int   cut_arena_reset(Cut_Arena *);

    // The circular memory just resets its index to 0 when it reached max size.
    typedef struct Cut_Circular_Mem {
        cut_u8 *buffer;
        cut_uptrsize size;
        cut_uptrsize allocated;
    } Cut_Circular_Mem;

    void *cut_circular_mem_allocate(cut_uptrsize size, Cut_Circular_Mem *buffer_mem);
    void cut_circular_mem_reset(Cut_Circular_Mem *buffer_mem);

    // Cut_Mem_Allocator is used to pass memory allocation strategies to dependencies so you have more control over how those dependencies perform memory operations.

    typedef struct Cut_Mem_Allocator {
        void *data;
        void *(*allocate)(cut_uptrsize size, void *allocator_data);
        void  (*free)(void *ptr, void *allocator_data);
        void *(*resize)(cut_uptrsize size, void *old_ptr, cut_uptrsize old_size, void *allocator_data);
    } Cut_Mem_Allocator;

    cut_inlinable void *cut_mem_allocate(cut_uptrsize size, Cut_Mem_Allocator *allocator);

    // Allocates memory for the given type.
    #define cut_T_allocate(Type, MemAllocatorPtr) (Type *)cut_mem_allocate(sizeof(Type), MemAllocatorPtr)

    // Allocates memory for an array.
    #define cut_arr_allocate(Type, Count, MemAllocatorPtr) (Type *)cut_mem_allocate(sizeof(Type) * Count, MemAllocatorPtr)

    cut_inlinable void cut_mem_free(void *data, Cut_Mem_Allocator *allocator);

    cut_inlinable void *cut_mem_resize(cut_uptrsize size, void *old_data, cut_uptrsize old_size, Cut_Mem_Allocator *allocator);

    #define cut_arr_resize(NewCount, OldData, OldCount, MemAllocatorPtr)   \
        cut_mem_resize(NewCount * sizeof(*(OldData)), OldData, OldCount * sizeof(Type), MemAllocatorPtr)

    cut_inlinable void *cut_mem_clone(void *data, cut_uptrsize size, Cut_Mem_Allocator *allocator);

    cut_inlinable Cut_u8Arrview cut_u8arrview_clone(Cut_u8Arrview view, Cut_Mem_Allocator *allocator);

    #if defined(CUT_MEMORY_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        #define arrview_is_same cut_arrview_is_same

        typedef Cut_Buffer_Arena Buffer_Arena;
        #define buffer_arena_allocate cut_buffer_arena_allocate
        #define buffer_arena_reset cut_buffer_arena_reset

        typedef Cut_Arena Arena;
        #define arena_create cut_arena_create
        #define arena_create_ cut_arena_create_
        #define arena_init cut_arena_init
        #define arena_destroy cut_arena_destroy
        #define arena_allocate cut_arena_allocate
        #define arena_reset cut_arena_reset

        typedef Cut_Circular_Mem Circular_Mem;
        #define circular_mem_allocate cut_circular_mem_allocate
        #define circular_mem_reset cut_circular_mem_reset

        typedef Cut_Mem_Allocator Mem_Allocator;

        #define mem_allocate cut_mem_allocate
        #define T_allocate cut_T_allocate
        #define arr_allocate cut_arr_allocate
        #define mem_free cut_mem_free
        #define mem_resize cut_mem_resize
        #define arr_resize cut_arr_resize
        #define mem_clone cut_mem_clone
        #define u8arrview_clone cut_u8arrview_clone
    #endif
#endif

#ifndef CUT_MEMORY_INL_IMPL_INCLUDED
    #define CUT_MEMORY_INL_IMPL_INCLUDED

    cut_inlinable void cut_buffer_arena_reset(Cut_Buffer_Arena *arena)
    {
        arena->allocated = 0;
    }

    cut_inlinable Cut_Arena cut_arena_create(cut_uptrsize size_to_reserve, cut_uptrsize commit_size, cut_u32 alignment)
    {
        Cut_Arena arena = {
            size_to_reserve,
            commit_size,
            alignment
        };
        cut_arena_init(&arena);
        return arena;
    }

    cut_inlinable void cut_circular_mem_reset(Cut_Circular_Mem *mem)
    {
        mem->allocated = 0;
    }

    cut_inlinable void *cut_mem_allocate(cut_uptrsize size, Cut_Mem_Allocator *allocator)
    {
        return allocator->allocate(size, allocator->data);
    }

    cut_inlinable void cut_mem_free(void *data, Cut_Mem_Allocator *allocator)
    {
        allocator->free(data, allocator->data);
    }

    cut_inlinable void *cut_mem_resize(cut_uptrsize size, void *old_data, cut_uptrsize old_size, Cut_Mem_Allocator *allocator)
    {
        return allocator->resize(size, old_data, old_size, allocator->data);
    }

    cut_inlinable void *cut_mem_clone(void *data, cut_uptrsize size, Cut_Mem_Allocator *allocator)
    {
        void *cloned = cut_mem_allocate(size, allocator);
        memcpy(cloned, data, size);
        return cloned;
    }

    cut_inlinable Cut_u8Arrview cut_u8arrview_clone(Cut_u8Arrview view, Cut_Mem_Allocator *allocator)
    {
        Cut_u8Arrview v;
        v.data = cut_mem_clone(view.data, view.count, allocator);
        v.count = view.count;
        return v;
    }
#endif

#if !defined(CUT_MEMORY_IMPL_INCLUDED) && (defined(CUT_MEMORY_IMPL) || defined(CUT_IMPL))
    #define CUT_MEMORY_IMPL_INCLUDED

    #if CUT_TARGET_OS == CUT_WINDOWS
        #include "win32.c"
    #elif CUT_TARGET_OS == CUT_MACOS || CUT_TARGET_OS == CUT_LINUX
        #include <sys/mman.h>
    #endif

    void *cut_buffer_arena_allocate(cut_uptrsize size, Cut_Buffer_Arena *arena)
    {
        cut_assert(arena->allocated + size <= arena->size);

        void *ptr = arena->buffer + arena->allocated;

        arena->allocated += size;

        return ptr;
    }

    void *cut_circular_mem_allocate(cut_uptrsize size, Cut_Circular_Mem *mem)
    {
        cut_assert(mem->size >= size);

        if (mem->allocated + size > mem->size)
            mem->allocated = 0;

        void *ptr = mem->buffer + mem->allocated;

        mem->allocated += size;

        return ptr;
    }

    int cut_arena_init(Cut_Arena *arena)
    {
        cut_assert(arena->reserved_size);
        cut_assert(arena->commit_size);

        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            arena->start = cut_WIN32_VirtualAlloc(
                0,
                arena->reserved_size,
                CUT_WIN32_MEM_RESERVE,
                CUT_WIN32_PAGE_READWRITE
            );
        }
        #elif CUT_TARGET_OS == CUT_MACOS || CUT_TARGET_OS == CUT_LINUX
        {
            arena->start = mmap(0, arena->reserved_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        }
        #endif

        arena->current = arena->start;
        arena->uncommitted_start = arena->start;
        arena->end = (cut_u8 *)arena->start + arena->reserved_size;

        return arena->start != 0;
    }

    void cut_arena_destroy(Cut_Arena *arena)
    {
        #if CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
            munmap(arena->start, arena->reserved_size);
        #elif CUT_TARGET_OS == CUT_WINDOWS
            cut_WIN32_VirtualFree(arena->start, 0, CUT_WIN32_MEM_RELEASE);
        #endif
    }

    void *cut_arena_allocate(cut_uptrsize size, Cut_Arena *arena)
    {
        cut_u8 *result = (cut_u8 *)cut_align_nb((cut_uptrsize)arena->current, arena->alignment);
        cut_u8 *end = result + size;

        if (end > arena->uncommitted_start) {
            cut_uptrsize size_to_commit;

            size_to_commit = (cut_uptrsize)(end - arena->uncommitted_start);
            size_to_commit = size_to_commit > arena->commit_size ?
                cut_align_nb(size_to_commit, arena->commit_size) : arena->commit_size;

            #if CUT_TARGET_OS == CUT_WINDOWS
            {
                void *committed = cut_WIN32_VirtualAlloc(
                    arena->uncommitted_start,
                    size_to_commit,
                    CUT_WIN32_MEM_COMMIT,
                    CUT_WIN32_PAGE_READWRITE
                );
                arena->uncommitted_start = committed;
            }
            #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
                arena->uncommitted_start += size_to_commit;
            #endif
        }

        arena->current = end;

        return result;
    }

    int cut_arena_reset(Cut_Arena *arena)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            int res = cut_WIN32_VirtualFree(arena->start, arena->reserved_size, CUT_WIN32_MEM_DECOMMIT);
        }
        #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
        {
            #ifdef MADV_DONTNEED
                madvise(arena->start, arena->reserved_size, MADV_DONTNEED);
            #endif
        }
        #endif

        arena->current = arena->start;
        arena->uncommitted_start = arena->start;

        return 1;
    }
#endif
