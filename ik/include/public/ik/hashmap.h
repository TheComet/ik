#ifndef WAVESIM_MAP_H
#define WAVESIM_MAP_H

#include "ik/config.h"
#include "ik/hash.h"

#define HM_SLOT_UNUSED    ((hash32_t)0)
#define HM_SLOT_RIP ((hash32_t)1)
#define HM_SLOT_INVALID   ((hash32_t)-1)
#define HM_REHASH_AT_PERCENT   70
#define HM_DEFAULT_TABLE_COUNT 128

C_BEGIN

struct hashmap_t
{
    hash32_t     table_count;
    hash32_t     key_size;
    hash32_t     value_size;
    hash32_t     slots_used;
    hash32_func  hash;
    void*        storage;
#ifdef IK_HASHMAP_STATS
    struct {
        uintptr_t total_insertions;
        uintptr_t total_deletions;
        uintptr_t total_tombstones;
        uintptr_t total_tombstone_reuses;
        uintptr_t total_rehashes;
        uintptr_t total_insertion_probes;
        uintptr_t total_deletion_probes;
        uintptr_t max_slots_used;
        uintptr_t max_slots_tombstoned;
        hash32_t current_tombstone_count;
    } stats;
#endif
};

/*!
 * @brief Allocates and initializes a new hashmap.
 * @param[out] hm A pointer to the new hashmap is written to this parameter.
 * Example:
 * ```cpp
 * struct hashmap_t* hm;
 * if (hashmap_create(&hm, sizeof(key_t), sizeof(value_t)) != IK_OK)
 *     handle_error();
 * ```
 * @param[in] key_size Specifies how many bytes of the "key" parameter to hash
 * in the hashmap_insert() call. Due to performance reasons, all keys are
 * identical in size. If you wish to use strings for keys, then you need to
 * specify the maximum possible string length here, and make sure you never
 * use strings that are longer than that (hashmap_insert_key contains a safety
 * check in debug mode for this case).
 * @note This parameter must be larger than 0.
 * @param[in] value_size Specifies how many bytes long the value type is. When
 * calling hashmap_insert(), value_size number of bytes are copied from the
 * memory pointed to by value into the hashmap.
 * @note This parameter may be 0.
 * @return If successful, returns IK_OK. If allocation fails,
 * IK_ERR_OUT_OF_MEMORY is returned.
 */
IK_PRIVATE_API ikret_t
hashmap_create(struct hashmap_t** hm, hash32_t key_size, hash32_t value_size);

/*!
 * @brief Initializes a new hashmap. See hashmap_create() for details on
 * parameters and return values.
 */
IK_PRIVATE_API ikret_t
hashmap_construct(struct hashmap_t* hm, hash32_t key_size, hash32_t value_size);

/*!
 * @brief Cleans up internal resources without freeing the hashmap object itself.
 */
IK_PRIVATE_API void
hashmap_destruct(struct hashmap_t* hm);

/*!
 * @brief Cleans up all resources and frees the hashmap.
 */
IK_PRIVATE_API void
hashmap_destroy(struct hashmap_t* hm);

/*!
 * @brief Inserts a key and value into the hashmap.
 * @note Complexity is generally O(1). Inserting may cause a rehash if the
 * table size exceeds HM_REHASH_AT_PERCENT.
 * @param[in] hm A pointer to a valid hashmap object.
 * @param[in] key A pointer to where the key is stored. key_size number of
 * bytes are hashed and copied into the hashmap from this location in
 * memory. See hashmap_create() on key_size.
 * @param[in] value A pointer to where the value is stored. value_size number
 * of bytes are copied from this location in memory into the hashmap. If
 * value_size is 0, then nothing is copied.
 * @return If the key already exists, then nothing is copied into the hashmap
 * and IK_HASH_EXISTS is returned. If the key is successfully inserted, IK_OK
 * is returned. If insertion failed, IK_ERR_OUT_OF_MEMORY is returned.
 */
IK_PRIVATE_API ikret_t
hashmap_insert(struct hashmap_t* hm, const void* key, const void* value);

IK_PRIVATE_API ikret_t
hashmap_insert_str(struct hashmap_t* hm, const char* key, const void* value);

IK_PRIVATE_API void*
hashmap_erase(struct hashmap_t* hm, const void* key);

IK_PRIVATE_API void*
hashmap_erase_str(struct hashmap_t* hm, const char* key);

IK_PRIVATE_API void*
hashmap_find(const struct hashmap_t* hm, const void* key);

IK_PRIVATE_API void*
hashmap_find_str(struct hashmap_t* hm, const char* key);

#define hashmap_count(hm) ((hm)->slots_used)

#define HASHMAP_FOR_EACH(hm, key_t, value_t, key, value) { \
    key_t* key; \
    value_t* value; \
    hash32_t pos_##value; \
    for (pos_##value = 0; \
        pos_##value != (hm)->table_count && \
            ((key = (key_t*)((uint8_t*)(hm)->storage + (sizeof(hash32_t) + (hm)->key_size) * pos_##value + sizeof(hash32_t))) || 1) && \
            ((value = (value_t*)((uint8_t*)(hm)->storage + (sizeof(hash32_t) + (hm)->key_size) * (hm)->table_count + (hm)->value_size * pos_##value)) || 1); \
        ++pos_##value) \
    { \
        hash32_t slot_##value = *(hash32_t*)((uint8_t*)(hm)->storage + (sizeof(hash32_t) + (hm)->key_size) * pos_##value); \
        if (slot_##value == HM_SLOT_UNUSED || slot_##value == HM_SLOT_RIP || slot_##value == HM_SLOT_INVALID) \
            continue; \
        {

#define HASHMAP_END_EACH }}}

C_END

#endif /* WAVESIM_MAP_H */
