#include "../dyn_array.c"

void run_dyn_array_tests()
{
    Dyn_Array arr = {
        .element_size = sizeof(int),
        .increment = 5,
        .mem_allocator = memallocator
    };

    dyn_array_init(&arr, 20);
    
    test(*(int *)dyn_array_add_back(&arr, &(int){5}) == 5);
    test(arr.count == 1);
    test(*(int *)dyn_array_at(&arr, 0) == 5);
    test(dyn_array_atv(&arr, int, 0) == 5);

    test(*(int *)dyn_array_add_back(&arr, &(int){10}) == 10);
    test(arr.count == 2);
    test(*(int *)dyn_array_at(&arr, 1) == 10);

    int i = 0;
    dyn_array_for(&arr, int *it, i++);
    test(i == arr.count);

    i = 0;
    dyn_array_for(&arr, int *it, { i++; }); // Alternative way for multiple instructions.
    test(i == arr.count);

    test(dyn_array_resize(&arr, 1));
    test(arr.capacity == 1);
    test(arr.count == 1);
    test(*(int *)dyn_array_at(&arr, 0) == 5);

    test(dyn_array_resize(&arr, 50));
    test(arr.capacity == 50);
    test(arr.count == 1);
    test(*(int *)dyn_array_at(&arr, 0) == 5);

    dyn_array_remove_back(&arr);
    test(arr.count == 0);

    dyn_array_deinit(&arr);
}
