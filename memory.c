#ifndef CUT_MEMORY
    #define CUT_MEMORY

    #include "base.c"

    // This thing is slow, it performs a byte per byte comparison.
    cut_inlinable int cut_mem_cmp(void *l, void *r, cut_u64 size);

    // Same as mem_cmp but returns 1 when identical not 0.
    cut_inlinable int cut_mem_is_same(void *l, void *r, cut_u64 size);

    #define cut_arrview_is_same(ArrViewPtr1, ArrViewPtr2)   \
        ((ArrViewPtr1)->count == (ArrViewPtr2)->count && cut_mem_is_same((ArrViewPtr1)->data, (ArrViewPtr2)->data, (ArrViewPtr2)->count))

    cut_inlinable void cut_mem_cpy(void *dest, void *src, cut_u64 size);

    cut_inlinable void cut_mem_set(void *dest, cut_u8 val, cut_u64 size);

    typedef struct Cut_Fixed_Buffer_Mem {
        cut_u8 *buffer;
        cut_u64 size;
        cut_u64 allocated;
    } Cut_Fixed_Buffer_Mem;

    cut_inlinable void *cut_fixed_buffer_mem_allocate(cut_u64 size, Cut_Fixed_Buffer_Mem *buffer_mem);
    cut_inlinable void cut_fixed_buffer_mem_reset(Cut_Fixed_Buffer_Mem *buffer_mem);

    // Cut_Virt_Mem is just a wrapper around Win32 VirtualAlloc/VirtualFree and Unix mmap/munmap.
    typedef struct Cut_Virt_Mem {
        cut_u64 reserved_size;
        cut_u64 commit_size;
        cut_u32 alignment;

        cut_u8 *start;
        cut_u8 *current;
        cut_u8 *uncommited_start;
        cut_u8 *end;
    } Cut_Virt_Mem;

    int   cut_virt_mem_init(Cut_Virt_Mem *);
    void  cut_virt_mem_destroy(Cut_Virt_Mem *);
    void *cut_virt_mem_allocate(cut_u64 size, Cut_Virt_Mem *);
    int   cut_virt_mem_reset(Cut_Virt_Mem *);

    // The circular memory just resets its index to 0 when it reached max size.
    typedef struct Cut_Circular_Mem {
        cut_u8 *buffer;
        cut_u64 size;
        cut_u64 allocated;
    } Cut_Circular_Mem;

    cut_inlinable void *cut_circular_mem_allocate(cut_u64 size, Cut_Circular_Mem *buffer_mem);
    cut_inlinable void cut_circular_mem_reset(Cut_Circular_Mem *buffer_mem);

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

    #define cut_arr_resize(Type, NewCount, OldCount, MemAllocatorPtr)   \
        (Type *)cut_mem_resize(NewCount * sizeof(Type), OldCount * sizeof(Type), MemAllocatorPtr)

    cut_inlinable void *cut_mem_clone(void *data, cut_uptrsize size, Cut_Mem_Allocator *allocator);

    cut_inlinable cut_u8Arrview cut_u8arrview_clone(cut_u8Arrview view, Cut_Mem_Allocator *allocator);

    #if defined(CUT_MEMORY_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        #define mem_cmp(l, r, size) cut_mem_cmp(l, r, size)
        #define mem_is_same(l, r, size) cut_mem_is_same(l, r, size)
        #define arrview_is_same cut_arrview_is_same
        #define mem_cpy(dest, src, size) cut_mem_cpy(dest, src, size)
        #define mem_set(DestPtr, Val, Size) cut_mem_set(DestPtr, Val, Size)

        typedef Cut_Mem_Allocator Mem_Allocator;

        #define mem_allocate(Size, MemAllocatorPtr) cut_mem_allocate(Size, MemAllocatorPtr)
        #define T_allocate(Type, MemAllocator) cut_T_allocate(Type, MemAllocator)
        #define arr_allocate(Type, Count, MemAllocator) cut_arr_allocate(Type, Count, MemAllocator)
        #define mem_resize(size, old_data, old_size, allocator) cut_mem_resize(size, old_data, old_size, allocator)
        #define arr_resize cut_arr_resize
        #define mem_free(data, allocator) cut_mem_free(data, allocator)
        #define mem_free_all(allocator) cut_mem_free_all(allocator)
        #define mem_init_allocator(allocator) cut_mem_init_allocator(allocator)
        #define mem_clone(data, size, allocator) cut_mem_clone(data, size, allocator)
        #define u8arrview_clone cut_u8arrview_clone

        typedef Cut_Fixed_Buffer_Mem Fixed_Buffer_Mem;
        #define fixed_buffer_mem_allocate(size, allocator_data) cut_fixed_buffer_mem_allocate(size, allocator_data)
        #define fixed_buffer_mem_resize(size, old, old_size, allocator_data) cut_fixed_buffer_mem_allocate(size, old, old_size, allocator_data)
        #define fixed_buffer_mem_reset(allocator_data) cut_fixed_buffer_mem_reset(allocator_data)

        typedef Cut_Virt_Mem Virt_Mem;
        #define virt_mem_init cut_virt_mem_init
        #define virt_mem_destroy cut_virt_mem_destroy
        #define virt_mem_allocate cut_virt_mem_allocate
        #define virt_mem_reset cut_virt_mem_reset

        typedef Cut_Circular_Mem Circular_Mem;
        #define circular_mem cut_circular_mem
        #define circular_mem_allocate cut_circular_mem_allocate
        #define circular_mem_reset cut_circular_mem_reset
    #endif
#endif

#ifndef CUT_MEMORY_INL_IMPL_INCLUDED
    #define CUT_MEMORY_INL_IMPL_INCLUDED

    cut_inlinable int cut_mem_cmp(void *l, void *r, cut_u64 size)
    {
        for (cut_u64 i = 0; i < size; i++) {
            if (((cut_u8*)l)[i] != ((cut_u8*)r)[i]) {
                return 1;
            }
        }
        return 0;
    }

    cut_inlinable int cut_mem_is_same(void *l, void *r, cut_u64 size)
    {
        return !cut_mem_cmp(l, r, size);
    }

    cut_inlinable void cut_mem_cpy(void *dest, void *src, cut_u64 size)
    {
        for (cut_u64 i = 0; i < size; i++) {
            ((cut_u8*)dest)[i] = ((cut_u8*)src)[i];
        }
    }

    cut_inlinable void cut_mem_set(void *dest, cut_u8 val, cut_u64 size)
    {
        for (cut_u64 i = 0; i < size; i++) {
            ((cut_u8*)dest)[i] = val;
        }
    }

    cut_inlinable void *cut_mem_allocate(cut_u64 size, Cut_Mem_Allocator *allocator)
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
        void *new = cut_mem_allocate(size, allocator);
        cut_mem_cpy(new, data, size);
        return new;
    }

    cut_inlinable cut_u8Arrview cut_u8arrview_clone(cut_u8Arrview view, Cut_Mem_Allocator *allocator)
    {
        cut_u8Arrview v;
        v.data = cut_mem_clone(view.data, view.count, allocator);
        v.count = view.count;
        return v;
    }

    cut_inlinable void *cut_fixed_buffer_mem_allocate(cut_uptrsize size, Cut_Fixed_Buffer_Mem *buffer_mem)
    {
        cut_assert(buffer_mem->allocated + size <= buffer_mem->size);

        void *ptr = buffer_mem->buffer + buffer_mem->allocated;

        buffer_mem->allocated += size;

        return ptr;
    }

    cut_inlinable void cut_fixed_buffer_mem_reset(Cut_Fixed_Buffer_Mem *buffer_mem)
    {
        buffer_mem->allocated = 0;
    }

    cut_inlinable void *cut_circular_mem_allocate(cut_u64 size, Cut_Circular_Mem *buffer_mem)
    {
        cut_assert(buffer_mem->size >= size);

        if (buffer_mem->allocated + size > buffer_mem->size)
            buffer_mem->allocated = 0;

        void *ptr = buffer_mem->buffer + buffer_mem->allocated;

        buffer_mem->allocated += size;

        return ptr;
    }

    cut_inlinable void cut_circular_mem_reset(Cut_Circular_Mem *buffer_mem)
    {
        buffer_mem->allocated = 0;
    }
#endif

#if !defined(CUT_MEMORY_IMPL_INCLUDED) && (defined(CUT_MEMORY_IMPL) || defined(CUT_IMPL))
    #define CUT_MEMORY_IMPL_INCLUDED

    #if CUT_TARGET_OS == CUT_WINDOWS
        #include "win32.c"
    #elif CUT_TARGET_OS == CUT_MACOS
        #include <sys/mman.h>
    #endif

    int cut_virt_mem_init(Cut_Virt_Mem *virt)
    {
        cut_assert(virt->reserved_size);
        cut_assert(virt->commit_size);

        #if CUT_TARGET_OS == CUT_WINDOWS
        {
            virt->start = cut_WIN32_VirtualAlloc(
                0,
                virt->reserved_size,
                CUT_WIN32_MEM_RESERVE,
                CUT_WIN32_PAGE_READWRITE
            );
            if (!virt->start)
                return 0;
        }
        #elif CUT_TARGET_OS == CUT_MACOS || CUT_TARGET_OS == CUT_LINUX
        {
            virt->start = mmap(0, virt->reserved_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        }
        #endif

        virt->current = virt->start;
        virt->uncommited_start = virt->start;
        virt->end = (cut_u8 *)virt->start + virt->reserved_size;

        return 1;
    }

    void cut_virt_mem_destroy(Cut_Virt_Mem *virt)
    {
        #if CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
            munmap(virt->start, virt->reserved_size);
        #elif CUT_TARGET_OS == CUT_WINDOWS
            cut_WIN32_VirtualFree(virt->start, 0, CUT_WIN32_MEM_RELEASE);
        #endif
    }

    void *cut_virt_mem_allocate(cut_u64 size, Cut_Virt_Mem *virt)
    {
        cut_u8 *result = (cut_u8 *)cut_align_nb((cut_u64)virt->current, virt->alignment);
        cut_u8 *end = result + size;

        if (end > virt->uncommited_start) {
            if (end > virt->end)   // @TODO: we should check only in debug mode ?
                return 0;

            cut_u64 size_to_commit;

            size_to_commit = (cut_u64)(end - virt->uncommited_start);
            size_to_commit = size_to_commit > virt->commit_size ?
                cut_align_nb(size_to_commit, virt->commit_size) : virt->commit_size;

            #if CUT_TARGET_OS == CUT_WINDOWS
                virt->uncommited_start = cut_WIN32_VirtualAlloc(
                    virt->uncommited_start,
                    size_to_commit,
                    CUT_WIN32_MEM_COMMIT,
                    CUT_WIN32_PAGE_READWRITE
                );
            #elif CUT_TARGET_OS == CUT_LINUX || CUT_TARGET_OS == CUT_MACOS
                virt->uncommited_start += size_to_commit;
            #endif
        }

        virt->current = end;

        return result;
    }

    int cut_virt_mem_reset(Cut_Virt_Mem *virt)
    {
        #if CUT_TARGET_OS == CUT_WINDOWS
            int res = cut_WIN32_VirtualFree(virt->start, virt->reserved_size, CUT_WIN32_MEM_DECOMMIT);
            if (!res)
                return 0;
        #endif

        virt->current = virt->start;
        virt->uncommited_start = virt->start;

        return 1;
    }
#endif
