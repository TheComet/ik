#ifndef WAVESIM_MAP_H
#define WAVESIM_MAP_H

#include "ik/config.h"
#include "ik/hash.h"

#define HM_SLOT_UNUSED    ((hash32_t)0)
#define HM_SLOT_TOMBSTONE ((hash32_t)1)
#define HM_SLOT_INVALID   ((hash32_t)-1)
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
};

IK_PRIVATE_API ikret_t
hashmap_create(struct hashmap_t** hm, hash32_t key_size, hash32_t value_size);

IK_PRIVATE_API ikret_t
hashmap_construct(struct hashmap_t* hm, hash32_t key_size, hash32_t value_size);

IK_PRIVATE_API void
hashmap_destruct(struct hashmap_t* hm);

IK_PRIVATE_API void
hashmap_destroy(struct hashmap_t* hm);

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
        if (slot_##value == HM_SLOT_UNUSED || slot_##value == HM_SLOT_TOMBSTONE || slot_##value == HM_SLOT_INVALID) \
            continue; \
        {

#define HASHMAP_END_EACH }}}

C_END

#endif /* WAVESIM_MAP_H */
