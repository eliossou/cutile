// This hash table is not automatically resized. You need to set the size beforehand by calling "init".
// It can be resized by calling "resize".
// The more available slots the hash table has, the faster it operates when adding new elements.
// Memory allocation failures are not checked, you need to handle this yourself.

#ifndef CUT_HASH_TABLE
    #define CUT_HASH_TABLE

    #include "base.c"
    #include "memory.c"

    typedef cut_u32 Cut_Hash_Table_Hash;

    typedef struct Cut_Hash_Table_Element {
        Cut_Hash_Table_Hash hash;
        cut_u8 kv[];
    } Cut_Hash_Table_Element;

    enum {
        CUT_HASH_TABLE_UNOCCUPIED_HASH  = 0,
        CUT_HASH_TABLE_REMOVED_HASH     = 1,
        CUT_HASH_TABLE_FIRST_VALID_HASH = 2
    };

    typedef struct Cut_Hash_Table {
        Cut_Hash_Table_Hash (*hash_proc)(void *key, cut_u64 key_size);
        int (*keys_are_same_proc)(void *key, void *other_key, cut_u64 key_size);

        cut_u16 key_size;
        cut_u16 value_size;

        Cut_Mem_Allocator mem_allocator;

        union {
            struct Cut_Hash_Table_Element *elements;
            cut_u8 *elements_u8;
        };

        cut_u32 count;
        cut_u32 capacity;
    } Cut_Hash_Table;

    // Just used as an hint to know what the hash table is supposed to contain.
    #define Cut_Hash_Table(TKey, TValue) Cut_Hash_Table

    #define cut_hash_table(HashProc, KeysSameProc, KeySize, ValueSize, MemAllocator)  \
        (Cut_Hash_Table){ HashProc, KeysSameProc, KeySize, ValueSize, MemAllocator }

    #define cut_hash_table_default(KeySize, ValueSize, MemAllocator)  \
        (Cut_Hash_Table){ cut_hash_table_hash, cut_hash_table_keys_are_same, KeySize, ValueSize, MemAllocator }

    #define cut_hash_table_default_u8arrview(ValueSize, MemAllocator)               \
        (Cut_Hash_Table) {                                                          \
            cut_hash_table_hash_u8arrview, cut_hash_table_keys_are_same_u8arrview,  \
            sizeof(u8arrview), ValueSize, MemAllocator                              \
        }

    Cut_Hash_Table_Hash cut_hash_table_hash(void *key, cut_u64 key_size);
    Cut_Hash_Table_Hash cut_hash_table_hash_u8arrview(void *key, cut_u64 key_size);

    int cut_hash_table_keys_are_same(void *key, void *other_key, cut_u64 key_size);
    int cut_hash_table_keys_are_same_u8arrview(void *key, void *other_key, cut_u64 key_size);

    void cut_hash_table_init(Cut_Hash_Table *ht, cut_u32 init_capacity);

    cut_inlinable void cut_hash_table_deinit(Cut_Hash_Table *ht)
    {
        cut_mem_free(ht->elements, &ht->mem_allocator);
    }

    // Trying to add a new key when the hash table is full results in an infinite loop.
    // If you try to add a already existing key, it will not replace the matching one.
    // Instead it will just try to find the first index available, making your hash table now having twice the same key.
    // So please, make sure all your keys are unique beforehand.
    // If you really need to know if your key is already present you can just use hash_table_get.
    // This choice has been made for performance reason, I do not want to compare the keys when adding new elements because in most
    // cases I do not deal with potential duplicates.
    void *cut_hash_table_add(Cut_Hash_Table *ht, void *key_ptr, void *value_ptr);
    void *cut_hash_table_add_empty(Cut_Hash_Table *ht, void *key_ptr);
    // This function resizes the hash table if needed. "threshold_factor" is the percent of occupied slots before resizing occurs. 75% is good.
    void *cut_hash_table_add_resize_factor(Cut_Hash_Table *ht, void *key_ptr, void *value_ptr, cut_u8 threshold_factor, cut_u32 increment);

    int cut_hash_table_remove(Cut_Hash_Table *ht, void *key);

    // Those functions return a null pointer when the key is not found so you can also use them for key search.
    void *cut_hash_table_get(Cut_Hash_Table *ht, void *key);    // Returns a pointer to the stored value.
    void *cut_hash_table_get_kv(Cut_Hash_Table *ht, void *key); // Returns a pointer to the key-value pair.

    // Exposed because it is used by cut_hash_table_for.
    Cut_Hash_Table_Element *cut_hash_table_get_element_at(Cut_Hash_Table *ht, cut_u32 index);

    void cut_hash_table_resize(Cut_Hash_Table *ht, cut_u32 new_capacity);

    cut_inlinable cut_u64 cut_hash_table_get_element_size(Cut_Hash_Table *ht)
    {
        cut_assert(ht->key_size);
        cut_assert(ht->value_size);
        return sizeof(struct Cut_Hash_Table_Element) + ht->key_size + ht->value_size;
    }

    cut_inlinable cut_u8 cut_hash_table_fill_factor(Cut_Hash_Table *ht)
    {
        cut_f32 factor = (cut_f32)ht->count / (cut_f32)ht->capacity;
        factor *= 100.f;

        return (cut_u8)factor;
    }

    #define cut_hash_table_for(HashTablePtr, ItKey, ItVal, Code)        \
        for (cut_u32 cut_i = 0, cut_j = 0;                              \
             cut_j < (HashTablePtr)->count;                             \
             cut_i++) {                                                 \
            struct Cut_Hash_Table_Element *cut_ht_it =                  \
                cut_hash_table_get_element_at((HashTablePtr), cut_i);   \
            if (cut_ht_it->hash >= CUT_HASH_TABLE_FIRST_VALID_HASH) {   \
                cut_j++;                                                \
                {                                                       \
                    ItKey = (void*)cut_ht_it->kv;                       \
                    ItVal = (void*)                                     \
                        (cut_ht_it->kv + (HashTablePtr)->key_size);     \
                    Code;                                               \
                }                                                       \
            }                                                           \
        }

    #if defined(CUT_HASH_TABLE_SHORT_NAMES) || defined(CUT_SHORT_NAMES)
        typedef Cut_Hash_Table_Element Hash_Table_Element;
        #define HASH_TABLE_UNOCCUPIED_HASH CUT_HASH_TABLE_UNOCCUPIED_HASH
        #define HASH_TABLE_FIRST_VALID_HASH CUT_HASH_TABLE_FIRST_VALID_HASH
        typedef Cut_Hash_Table Hash_Table;
        #define Hash_Table(TKey, TValue) Cut_Hash_Table(TKey, TValue)
        #define hash_table cut_hash_table
        #define hash_table_default cut_hash_table_default
        #define hash_table_default_u8arrview cut_hash_table_default_u8arrview
        #define Hash_Table_Hash Cut_Hash_Table_Hash
        #define hash_table_hash cut_hash_table_hash
        #define hash_table_hash_u8arrview cut_hash_table_hash_u8arrview
        #define hash_table_keys_are_same cut_hash_table_keys_are_same
        #define hash_table_keys_are_same_u8arrview cut_hash_table_keys_are_same_u8arrview
        #define hash_table_add(ht, key, value) cut_hash_table_add(ht, key, value)
        #define hash_table_add_resize_factor cut_hash_table_add_resize_factor
        #define hash_table_add_empty cut_hash_table_add_empty
        #define hash_table_remove(ht, key) cut_hash_table_remove(ht, key)
        #define hash_table_get(ht, key) cut_hash_table_get(ht, key)
        #define hash_table_get_kv(HashTablePtr, KeyPtr) cut_hash_table_get_kv(HashTablePtr, KeyPtr)
        #define hash_table_resize(ht, capacity) cut_hash_table_resize(ht, capacity)
        #define hash_table_init(ht, capacity) cut_hash_table_init(ht, capacity)
        #define hash_table_deinit(ht) cut_hash_table_deinit(ht)
        #define hash_table_for(HashTablePtr, ItKey, ItValue, Code) cut_hash_table_for(HashTablePtr, ItKey, ItValue, Code)
    #endif
#endif

#if !defined(CUT_HASH_TABLE_IMPL_INCLUDED) && (defined(CUT_HASH_TABLE_IMPL) || defined(CUT_IMPL))
    #define CUT_HASH_TABLE_IMPL_INCLUDED

    Cut_Hash_Table_Hash cut_hash_table_hash(void *key, cut_u64 key_size)
    {
        Cut_Hash_Table_Hash result;

        // FNV-32
        cut_u32 FNV_32_PRIME = 16777619;
        cut_u32 h = 2166136261;

        int i = 0;
        while (i < key_size) {
            h ^= ((cut_u8*)key)[i];

            /* multiply by the 32 bit FNV magic prime mod 2^32 */
            h *= FNV_32_PRIME;

            i++;
        }

        return h;
    }

    Cut_Hash_Table_Hash cut_hash_table_hash_u8arrview(void *key, cut_u64 key_size)
    {
        cut_u8arrview *v = (cut_u8arrview *)key;
        return cut_hash_table_hash(v->data, v->count);
    }

    int cut_hash_table_keys_are_same(void *key, void *other_key, cut_u64 key_size)
    {
        return cut_mem_is_same(key, other_key, key_size);
    }

    int cut_hash_table_keys_are_same_u8arrview(void *key, void *other_key, cut_u64 key_size)
    {
        cut_u8arrview *l = (cut_u8arrview *)key;
        cut_u8arrview *r = (cut_u8arrview *)other_key;

        return l->count == r->count && cut_mem_is_same(l->data, r->data, l->count);
    }

    cut_inlinable cut_u64 cut_hash_table_get_element_u8_position(Cut_Hash_Table *ht, cut_u32 index)
    {
        cut_u64 elem_size = cut_hash_table_get_element_size(ht);

        return index * elem_size;
    }

    Cut_Hash_Table_Element *cut_hash_table_get_element_at(Cut_Hash_Table *ht, cut_u32 index)
    {
        return (struct Cut_Hash_Table_Element *)
            (ht->elements_u8 + cut_hash_table_get_element_u8_position(ht, index));
    }

    void cut_hash_table_init(Cut_Hash_Table *ht, cut_u32 init_capacity)
    {
        cut_u64 elem_size = cut_hash_table_get_element_size(ht);

        ht->elements = cut_mem_allocate(elem_size * init_capacity, &ht->mem_allocator);

        for (cut_u32 i = 0; i < init_capacity; i++) {
            struct Cut_Hash_Table_Element *element = (struct Cut_Hash_Table_Element *)
                (((cut_u8 *)ht->elements) + cut_hash_table_get_element_u8_position(ht, i));

            element->hash = CUT_HASH_TABLE_UNOCCUPIED_HASH;
        }

        ht->count = 0;
        ht->capacity = init_capacity;
    }

    void cut_hash_table_resize(Cut_Hash_Table *ht, cut_u32 new_capacity)
    {
        cut_u64 elem_size = cut_hash_table_get_element_size(ht);

        Cut_Hash_Table new = {
            .hash_proc = ht->hash_proc,
            .keys_are_same_proc = ht->keys_are_same_proc,
            .key_size = ht->key_size,
            .value_size = ht->value_size,
            .mem_allocator = ht->mem_allocator
        };
        cut_hash_table_init(&new, new_capacity);

        cut_u32 to_copy = new.capacity < ht->capacity ? new.capacity : ht->capacity;

        Cut_Hash_Table_Element *element;
        for (cut_uptrsize i = 0; i < ht->capacity; i++) {
            if (new.count == new.capacity)
               break;

            element = cut_hash_table_get_element_at(ht, i);

            if (element->hash > CUT_HASH_TABLE_UNOCCUPIED_HASH)
                cut_hash_table_add(&new, element->kv, ((cut_u8 *)element->kv) + ht->key_size);
        }

        cut_hash_table_deinit(ht);
        *ht = new;
    }

    #define cut_hash_table_travel(Code)                                 \
        Cut_Hash_Table_Hash hash = ht->hash_proc(key, ht->key_size);    \
        if (hash < CUT_HASH_TABLE_FIRST_VALID_HASH)                     \
            hash += CUT_HASH_TABLE_FIRST_VALID_HASH;                    \
                                                                        \
        cut_u32 index = hash % ht->capacity;                            \
                                                                        \
        struct Cut_Hash_Table_Element *element;                         \
        cut_u32 probe_inc = 1;                                          \
                                                                        \
        loop:                                                           \
                                                                        \
        element = cut_hash_table_get_element_at(ht, index);             \
        if (element->hash > CUT_HASH_TABLE_UNOCCUPIED_HASH) {           \
            Code;                                                       \
                                                                        \
            index = (index+probe_inc) % ht->capacity;                   \
                                                                        \
            probe_inc++;                                                \
                                                                        \
            goto loop;                                                  \
        }                                                               \
                                                                        \
        end_walk:

    void *cut_hash_table_add(Cut_Hash_Table *ht, void *key, void *value)
    {
        void *dest = cut_hash_table_add_empty(ht, key);

        // Copies value.
        cut_mem_cpy(
            dest,
            value,
            ht->value_size
        );

        return dest;
    }

    void *cut_hash_table_add_empty(Cut_Hash_Table *ht, void *key)
    {
        cut_hash_table_travel({
            if (element->hash == CUT_HASH_TABLE_REMOVED_HASH)
                goto end_walk;
        });

        element->hash = hash;

        // Copies key.
        cut_u8 *dest = element->kv;
        cut_mem_cpy(
            dest,
            key,
            ht->key_size
        );

        dest += ht->key_size;

        ht->count++;

        return dest;
    }

    void *cut_hash_table_add_resize_factor(Cut_Hash_Table *ht, void *key_ptr, void *value_ptr, cut_u8 threshold_factor, cut_u32 increment)
    {
        cut_u8 fill_factor = cut_hash_table_fill_factor(ht);
        if (fill_factor >= threshold_factor)
            cut_hash_table_resize(ht, ht->capacity + increment);

        return cut_hash_table_add(ht, key_ptr, value_ptr);
    }

    int cut_hash_table_remove(Cut_Hash_Table *ht, void *key)
    {
        cut_hash_table_travel({
            if (element->hash == hash) {
                if (ht->keys_are_same_proc(element->kv, key, ht->key_size)) {
                    element->hash = CUT_HASH_TABLE_REMOVED_HASH;
                    ht->count -= 1;
                    return 1;
                }
            }
        });

        return 0;
    }

    void *cut_hash_table_get(Cut_Hash_Table *ht, void *key)
    {
        cut_hash_table_travel({
            if (element->hash >= CUT_HASH_TABLE_FIRST_VALID_HASH) {
                if (ht->keys_are_same_proc(element->kv, key, ht->key_size)) {
                    return element->kv + ht->key_size;
                }
            }
        });

        return 0;
    }

    void *cut_hash_table_get_kv(Cut_Hash_Table *ht, void *key)
    {
        cut_hash_table_travel({
            if (element->hash >= CUT_HASH_TABLE_FIRST_VALID_HASH) {
                if (ht->keys_are_same_proc(element->kv, key, ht->key_size)) {
                    return element->kv;
                }
            }
        });

        return 0;
    }
#endif
