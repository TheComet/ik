#include "ik/bstv.h"
#include "ik/memory.h"
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
struct bstv_t*
bstv_create(void)
{
    struct bstv_t* bstv;
    if (!(bstv = MALLOC(sizeof *bstv)))
        return NULL;
    bstv_construct(bstv);
    return bstv;
}

/* ------------------------------------------------------------------------- */
void
bstv_construct(struct bstv_t* bstv)
{
    assert(bstv);
    vector_construct(&bstv->vector, sizeof(bstv_hash_value_t));
}

/* ------------------------------------------------------------------------- */
void
bstv_destroy(struct bstv_t* bstv)
{
    assert(bstv);
    bstv_clear_free(bstv);
    FREE(bstv);
}

/* ------------------------------------------------------------------------- */
/* algorithm taken from GNU GCC stdlibc++'s lower_bound function, line 2121 in stl_algo.h */
/* https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a02014.html */
static bstv_hash_value_t*
bstv_find_lower_bound(const struct bstv_t* bstv, uint32_t hash)
{
    uint32_t half;
    bstv_hash_value_t* middle;
    bstv_hash_value_t* data;
    uint32_t len;

    assert(bstv);

    data = (bstv_hash_value_t*)bstv->vector.data;
    len = bstv->vector.count;

    /* if (the vector has no data, return NULL */
    if (!len)
        return NULL;

    while (len > 0)
    {
        half = len >> 1;
        middle = data + half;
        if (middle->hash < hash)
        {
            data = middle;
            ++data;
            len = len - half - 1;
        }
        else
            len = half;
    }

    /* if ("data" is pointing outside of the valid elements in the vector, also return NULL */
    if ((intptr_t)data >= (intptr_t)bstv->vector.data + (intptr_t)bstv->vector.count * (intptr_t)bstv->vector.element_size)
        return NULL;
    else
        return data;
}

/* ------------------------------------------------------------------------- */
ikret_t
bstv_insert(struct bstv_t* bstv, uint32_t hash, void* value)
{
    bstv_hash_value_t* emplaced_data;
    bstv_hash_value_t* lower_bound;

    assert(bstv);

    /* don't insert reserved hashes */
    if (hash == BSTV_INVALID_HASH)
        return IK_HASH_RESERVED;

    /* lookup location in bstv to insert */
    lower_bound = bstv_find_lower_bound(bstv, hash);
    if (lower_bound && lower_bound->hash == hash)
        return IK_HASH_EXISTS;

    /* either push back or insert, depending on whether there is already data
     * in the bstv */
    if (!lower_bound)
        emplaced_data = (bstv_hash_value_t*)vector_push_emplace(&bstv->vector);
    else
        emplaced_data = vector_insert_emplace(&bstv->vector,
                          lower_bound - (bstv_hash_value_t*)bstv->vector.data);

    if (!emplaced_data)
        return IK_RAN_OUT_OF_MEMORY;

    memset(emplaced_data, 0, sizeof *emplaced_data);
    emplaced_data->hash = hash;
    emplaced_data->value = value;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
bstv_set(struct bstv_t* bstv, uint32_t hash, void* value)
{
    bstv_hash_value_t* data;

    assert(bstv);

    data = bstv_find_lower_bound(bstv, hash);
    if (data && data->hash == hash)
        data->value = value;
}

/* ------------------------------------------------------------------------- */
void*
bstv_find(const struct bstv_t* bstv, uint32_t hash)
{
    void** result = bstv_find_ptr(bstv, hash);
    return result == NULL ? NULL : *result;
}

/* ------------------------------------------------------------------------- */
void**
bstv_find_ptr(const struct bstv_t* bstv, uint32_t hash)
{
    bstv_hash_value_t* data;

    assert(bstv);

    data = bstv_find_lower_bound(bstv, hash);
    if (!data || data->hash != hash)
        return NULL;
    return &data->value;
}

/* ------------------------------------------------------------------------- */
uint32_t
bstv_find_element(const struct bstv_t* bstv, const void* value)
{
    assert(bstv);

    VECTOR_FOR_EACH(&bstv->vector, bstv_hash_value_t, kv)
        if (kv->value == value)
            return kv->hash;
    VECTOR_END_EACH
    return BSTV_INVALID_HASH;
}

/* ------------------------------------------------------------------------- */
void*
bstv_get_any_element(const struct bstv_t* bstv)
{
    bstv_hash_value_t* kv;
    assert(bstv);
    kv = (bstv_hash_value_t*)vector_back(&bstv->vector);
    if (kv)
        return kv->value;
    return NULL;
}

/* ------------------------------------------------------------------------- */
ikret_t
bstv_hash_exists(struct bstv_t* bstv, uint32_t hash)
{
    bstv_hash_value_t* data;

    assert(bstv);

    data = bstv_find_lower_bound(bstv, hash);
    if (data && data->hash == hash)
        return IK_OK;
    return IK_HASH_NOT_FOUND;
}

/* ------------------------------------------------------------------------- */
uint32_t
bstv_find_unused_hash(struct bstv_t* bstv)
{
    uint32_t i = 0;

    assert(bstv);

    BSTV_FOR_EACH(bstv, void, key, value)
        if (i != key)
            break;
        ++i;
    BSTV_END_EACH
    return i;
}

/* ------------------------------------------------------------------------- */
void*
bstv_erase(struct bstv_t* bstv, uint32_t hash)
{
    void* value;
    bstv_hash_value_t* data;

    assert(bstv);

    data = bstv_find_lower_bound(bstv, hash);
    if (!data || data->hash != hash)
        return NULL;

    value = data->value;
    vector_erase_element(&bstv->vector, (uint8_t*)data);
    return value;
}

/* ------------------------------------------------------------------------- */
void*
bstv_erase_element(struct bstv_t* bstv, void* value)
{
    void* data;
    uint32_t hash;

    assert(bstv);

    hash = bstv_find_element(bstv, value);
    if (hash == BSTV_INVALID_HASH)
        return NULL;

    data = bstv_find_lower_bound(bstv, hash);
    vector_erase_element(&bstv->vector, (uint8_t*)data);

    return value;
}

/* ------------------------------------------------------------------------- */
void
bstv_clear(struct bstv_t* bstv)
{
    assert(bstv);
    vector_clear(&bstv->vector);
}

/* ------------------------------------------------------------------------- */
void bstv_clear_free(struct bstv_t* bstv)
{
    assert(bstv);
    vector_clear_free(&bstv->vector);
}
