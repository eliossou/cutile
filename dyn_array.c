// Memory allocation failures are not checked, you need to handle this yourself.

#ifndef CUT_DYN_ARRAY
    #define CUT_DYN_ARRAY

    #include "base.c"
    #include "memory.c"

    typedef struct Cut_Dyn_Array {
        cut_u32 element_size;
        cut_u32 increment;

        Cut_Mem_Allocator mem_allocator;

        cut_u32 count;
        cut_u32 capacity;

        void *data;
    } Cut_Dyn_Array;

    #define Cut_Dyn_Array(T) Cut_Dyn_Array

    #define cut_dyn_array(ElemSize, Increment, MemAllocator)   \
        (Cut_Dyn_Array){ .element_size = ElemSize, .increment = Increment, .mem_allocator = MemAllocator }

    cut_inlinable void cut_dyn_array_init(Cut_Dyn_Array *arr, cut_u32 init_capacity)
    {
        arr->data = cut_mem_allocate(init_capacity * arr->element_size, &arr->mem_allocator);
        arr->capacity = init_capacity;
    }

    cut_inlinable Cut_Dyn_Array cut_dyn_array_create(cut_u32 elem_size, cut_u32 increment, cut_u32 init_capacity, Cut_Mem_Allocator mem_allocator)
    {
        Cut_Dyn_Array arr = cut_dyn_array(elem_size, increment, mem_allocator);

        cut_dyn_array_init(&arr, init_capacity);

        return arr;
    }

    cut_inlinable void cut_dyn_array_deinit(Cut_Dyn_Array *arr)
    {
        cut_mem_free(arr->data, &arr->mem_allocator);
    }

    cut_inlinable int cut_dyn_array_resize(Cut_Dyn_Array *arr, cut_u64 new_capacity)
    {
        void *new = cut_mem_resize(
            arr->element_size * new_capacity,
            arr->data,
            arr->element_size * arr->capacity,
            &arr->mem_allocator
        );

        arr->data = new;

        arr->capacity = new_capacity;

        if (arr->count > arr->capacity)
            arr->count = arr->capacity;

        return 1;
    }

    cut_inlinable void *cut_dyn_array_at(Cut_Dyn_Array *arr, cut_u32 index)
    {
        cut_assert(arr->count > index);

        return (cut_u8 *)arr->data + (arr->element_size * index);
    }

    #define cut_dyn_array_atv(DynArrayPtr, Type, Index) (           \
        cut_assert(sizeof(Type) == (DynArrayPtr)->element_size),    \
        *((Type *)cut_dyn_array_at(DynArrayPtr, Index))             \
    )

    cut_inlinable void *cut_dyn_array_add_back_empty(Cut_Dyn_Array *arr)
    {
        if (arr->count == arr->capacity)
            cut_dyn_array_resize(arr, arr->capacity + arr->increment);

        void *result = cut_dyn_array_at(arr, arr->count);
        arr->count++;
        return result;
    }

    cut_inlinable void *cut_dyn_array_add_back(Cut_Dyn_Array *arr, void *val)
    {
        void *data = cut_dyn_array_add_back_empty(arr);
        memcpy(data, val, arr->element_size);  // @TODO: cut_mem_cpy is slow.
        return data;
    }

    cut_inlinable void *cut_dyn_array_add_back_many_empty(Cut_Dyn_Array *arr, cut_u32 count)
    {
        if (arr->count + count >= arr->capacity)
            cut_dyn_array_resize(arr, arr->capacity + cut_MAX(count, arr->increment));

        void *result = cut_dyn_array_at(arr, arr->count);
        arr->count += count;
        return result;
    }

    cut_inlinable void *cut_dyn_array_add_back_many(Cut_Dyn_Array *arr, void *vals, cut_u32 count)
    {
        void *result = cut_dyn_array_add_back_many_empty(arr, count);

        memcpy(result, vals, arr->element_size * count);

        return result;
    }

    cut_inlinable void cut_dyn_array_remove_back(Cut_Dyn_Array *arr)
    {
        cut_assert(arr->count);

        arr->count--;
    }

    cut_inlinable void *cut_dyn_array_back(Cut_Dyn_Array *arr)
    {
        cut_assert(arr->count);

        return cut_dyn_array_at(arr, arr->count - 1);
    }

    cut_inlinable cut_u32 cut_dyn_array_back_index(Cut_Dyn_Array *arr)
    {
        cut_assert(arr->count);

        return arr->count - 1;
    }

    cut_inlinable void cut_dyn_array_set_all(Cut_Dyn_Array *arr, void *val)
    {
        for (cut_u32 i = 0; i < arr->count; i++) {
            memcpy((cut_u8 *)arr->data + (i * arr->element_size), val, arr->element_size);
        }
    }

    #define cut_dyn_array_for(DynArrayPtr, ItDecl, Code)                    \
        for (cut_u32 cut_i = 0; cut_i < (DynArrayPtr)->count; cut_i++) {    \
            ItDecl = cut_dyn_array_at(DynArrayPtr, cut_i);                  \
            {                                                               \
                Code;                                                       \
            }                                                               \
        }

    cut_inlinable void cut_dyn_array_clear(Cut_Dyn_Array *arr)
    {
        arr->count = 0;
    }

    cut_inlinable void cut_dyn_array_remove_at(Cut_Dyn_Array *arr, cut_u32 index)
    {
        cut_assert(arr->count > index);

        cut_u32 to_copy = arr->count - index - 1;
        memcpy(
            (cut_u8 *)arr->data + (index * arr->element_size),
            (cut_u8 *)arr->data + ((index + 1) * arr->element_size),
            arr->element_size * to_copy
        );
        arr->count -= 1;
    }

    #if defined(CUT_DYN_ARRAY_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        typedef Cut_Dyn_Array Dyn_Array;
        #define Dyn_Array(T) Cut_Dyn_Array(T)
        #define dyn_array cut_dyn_array
        #define dyn_array_init cut_dyn_array_init
        #define dyn_array_create cut_dyn_array_create
        #define dyn_array_deinit cut_dyn_array_deinit
        #define dyn_array_resize cut_dyn_array_resize
        #define dyn_array_at cut_dyn_array_at
        #define dyn_array_atv cut_dyn_array_atv
        #define dyn_array_add_back_empty cut_dyn_array_add_back_empty
        #define dyn_array_add_back cut_dyn_array_add_back
        #define dyn_array_add_back_many_empty cut_dyn_array_add_back_many_empty
        #define dyn_array_add_back_many cut_dyn_array_add_back_many
        #define dyn_array_remove_back cut_dyn_array_remove_back
        #define dyn_array_back cut_dyn_array_back
        #define dyn_array_back_index cut_dyn_array_back_index
        #define dyn_array_for cut_dyn_array_for
        #define dyn_array_set_all cut_dyn_array_set_all
        #define dyn_array_clear cut_dyn_array_clear
        #define dyn_array_remove_at cut_dyn_array_remove_at
    #endif
#endif
