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

    // add_back_many
    {
        Dyn_Array arr2 = {
            .element_size = sizeof(int),
            .increment = 5,
            .mem_allocator = memallocator
        };
        dyn_array_init(&arr2, 2);

        int vals[] = {10, 20, 30};
        dyn_array_add_back_many(&arr2, vals, 3);
        test(arr2.count == 3);
        test(*(int *)dyn_array_at(&arr2, 0) == 10);
        test(*(int *)dyn_array_at(&arr2, 2) == 30);

        dyn_array_deinit(&arr2);
    }

    // remove_at
    {
        Dyn_Array arr3 = {
            .element_size = sizeof(int),
            .increment = 5,
            .mem_allocator = memallocator
        };
        dyn_array_init(&arr3, 10);
        dyn_array_add_back(&arr3, &(int){0});
        dyn_array_add_back(&arr3, &(int){1});
        dyn_array_add_back(&arr3, &(int){2});
        dyn_array_add_back(&arr3, &(int){3});
        dyn_array_add_back(&arr3, &(int){4});

        dyn_array_remove_at(&arr3, 2);
        test(arr3.count == 4);
        test(*(int *)dyn_array_at(&arr3, 0) == 0);
        test(*(int *)dyn_array_at(&arr3, 1) == 1);
        test(*(int *)dyn_array_at(&arr3, 2) == 3);
        test(*(int *)dyn_array_at(&arr3, 3) == 4);

        dyn_array_deinit(&arr3);
    }

    // clear and back
    {
        Dyn_Array arr4 = {
            .element_size = sizeof(int),
            .increment = 5,
            .mem_allocator = memallocator
        };
        dyn_array_init(&arr4, 10);
        dyn_array_add_back(&arr4, &(int){7});
        dyn_array_add_back(&arr4, &(int){8});
        dyn_array_add_back(&arr4, &(int){9});

        test(*(int *)dyn_array_back(&arr4) == 9);
        test(dyn_array_back_index(&arr4) == 2);

        dyn_array_clear(&arr4);
        test(arr4.count == 0);

        dyn_array_deinit(&arr4);
    }
}
