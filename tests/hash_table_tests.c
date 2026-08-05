#include "../hash_table.c"

void run_hash_table_tests()
{
    Hash_Table(u8Arrview, int) ht = hash_table_default_u8arrview(
        .value_size = sizeof(int),
        .mem_allocator = memallocator
    );

    // You can use init function to set a base size.
    hash_table_init(&ht, 30);

    int val = 1;
    test(*(int*)hash_table_add(&ht, &fstr0("One"), &val) == 1);
    val = 2;
    test(*(int*)hash_table_add(&ht, &fstr0("Two"), &val) == 2);
    val = 3;
    test(*(int*)hash_table_add(&ht, &fstr0("Twenty"), &val) == 3);

    test(ht.count == 3);

    test(*(int*)hash_table_get(&ht, &fstr0("One")) == 1);
    test(*(int*)hash_table_get(&ht, &fstr0("Two")) == 2);
    test(*(int*)hash_table_get(&ht, &fstr0("Twenty")) == 3);

    hash_table_resize(&ht, 14);

    test(*(int*)hash_table_get(&ht, &fstr0("One")) == 1);
    test(*(int*)hash_table_get(&ht, &fstr0("Two")) == 2);
    test(*(int*)hash_table_get(&ht, &fstr0("Twenty")) == 3);

    val = 5;
    test(ht.count == 3);

    test(*(int*)hash_table_get(&ht, &fstr0("One")) == 1);
    test(*(int*)hash_table_get(&ht, &fstr0("Two")) == 2);
    test(*(int*)hash_table_get(&ht, &fstr0("Twenty")) == 3);

    test(hash_table_remove(&ht, &fstr0("Two")));
    test(ht.count == 2);

    int *new_val = hash_table_add(&ht, &fstr0("Hello"), &(int){36});
    test(*new_val == 36);
    test(ht.count == 3);

    test(hash_table_get(&ht, &fstr0("NotExisting")) == 0);

    hash_table_deinit(&ht);

    Hash_Table(int, int) ht2 = {
        .hash_proc = hash_table_hash,
        .keys_are_same_proc = hash_table_keys_are_same,
        .key_size = sizeof(int),
        .value_size = sizeof(int),
        .mem_allocator = memallocator
    };

    hash_table_init(&ht2, 1024);

    for (int i = 0; i < 512; i++) {
        test(*(int*)hash_table_add(&ht2, &i, &i) == i);
    }

    test(ht2.count == 512);

    hash_table_resize(&ht2, 900);

    test(ht2.count == 512);

    test(hash_table_get(&ht2, &(int){513}) == 0);

    test(*(int*)hash_table_get(&ht2, &(int){325}) == 325);

    int i = 0;
    hash_table_for(&ht2, u8 *key, int *val, i++);
    test(i == ht2.count);

    i = 0;
    hash_table_for(&ht2, u8 *key, int *val, { i++; }); // Alternative for multiple instructions.
    test(i == ht2.count);

    hash_table_deinit(&ht2);
}
