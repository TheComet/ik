#include "ik/btree.h"
#include "ik/memory.h"
#include <assert.h>
#include <string.h>

const hash32_t BTREE_VECTOR_INVALID_HASH = (hash32_t)-1;

/* ------------------------------------------------------------------------- */
ikret_t
btree_create(struct btree_t** btree)
{
    *btree = MALLOC(sizeof **btree);
    if (*btree == NULL)
        return IK_ERR_OUT_OF_MEMORY;
    btree_construct(*btree);
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
btree_construct(struct btree_t* btree)
{
    assert(btree);
    vector_construct(&btree->vector, sizeof(struct btree_hash_value_t));
}

/* ------------------------------------------------------------------------- */
void
btree_destroy(struct btree_t* btree)
{
    assert(btree);
    btree_clear_free(btree);
    FREE(btree);
}

/* ------------------------------------------------------------------------- */
/* algorithm taken from GNU GCC stdlibc++'s lower_bound function, line 2121 in stl_algo.h */
/* https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a02014.html */
static struct btree_hash_value_t*
btree_find_lower_bound(const struct btree_t* btree, hash32_t hash)
{
    uintptr_t half;
    struct btree_hash_value_t* middle;
    struct btree_hash_value_t* data;
    uintptr_t len;

    assert(btree);

    data = ( struct btree_hash_value_t*)btree->vector.data;
    len = btree->vector.count;

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
    if ((uintptr_t)data >= (uintptr_t)btree->vector.data + btree->vector.count * btree->vector.element_size)
        return NULL;
    else
        return data;
}

/* ------------------------------------------------------------------------- */
ikret_t
btree_insert(struct btree_t* btree, hash32_t hash, void* value)
{
    struct btree_hash_value_t* emplaced_data;
    struct btree_hash_value_t* lower_bound;

    assert(btree);

    /* don't insert reserved hashes */
    if (hash == BTREE_VECTOR_INVALID_HASH)
        return IK_ERR_INVALID_HASH;

    /* lookup location in btree to insert */
    lower_bound = btree_find_lower_bound(btree, hash);
    if (lower_bound && lower_bound->hash == hash)
        return IK_HASH_EXISTS;

    /* either push back or insert, depending on whether there is already data
     * in the btree */
    if (!lower_bound)
        emplaced_data = (struct btree_hash_value_t*)vector_emplace(&btree->vector);
    else
        emplaced_data = vector_insert_emplace(&btree->vector,
            (uintptr_t)(lower_bound - (struct btree_hash_value_t*)btree->vector.data));

    if (!emplaced_data)
        return IK_ERR_OUT_OF_MEMORY;

    memset(emplaced_data, 0, sizeof *emplaced_data);
    emplaced_data->hash = hash;
    emplaced_data->value = value;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
btree_set(struct btree_t* btree, hash32_t hash, void* value)
{
    struct btree_hash_value_t* data;

    assert(btree);

    data = btree_find_lower_bound(btree, hash);
    if (data && data->hash == hash)
        data->value = value;
}

/* ------------------------------------------------------------------------- */
void*
btree_find(const struct btree_t* btree, hash32_t hash)
{
    void** result = btree_find_ptr(btree, hash);
    return result == NULL ? NULL : *result;
}

/* ------------------------------------------------------------------------- */
void**
btree_find_ptr(const struct btree_t* btree, hash32_t hash)
{
    struct btree_hash_value_t* data;

    assert(btree);

    data = btree_find_lower_bound(btree, hash);
    if (!data || data->hash != hash)
        return NULL;
    return &data->value;
}

/* ------------------------------------------------------------------------- */
hash32_t
btree_find_element(const struct btree_t* btree, const void* value)
{
    assert(btree);

    VECTOR_FOR_EACH(&btree->vector, struct btree_hash_value_t, kv)
        if (kv->value == value)
            return kv->hash;
    VECTOR_END_EACH
    return BTREE_VECTOR_INVALID_HASH;
}

/* ------------------------------------------------------------------------- */
void*
btree_get_any_element(const struct btree_t* btree)
{
    struct btree_hash_value_t* kv;
    assert(btree);
    kv = (struct btree_hash_value_t*)vector_back(&btree->vector);
    if (kv)
        return kv->value;
    return NULL;
}

/* ------------------------------------------------------------------------- */
int
btree_hash_exists(struct btree_t* btree, hash32_t hash)
{
    struct btree_hash_value_t* data;

    assert(btree);

    data = btree_find_lower_bound(btree, hash);
    if (data && data->hash == hash)
        return 0;
    return -1;
}

/* ------------------------------------------------------------------------- */
hash32_t
btree_find_unused_hash(struct btree_t* btree)
{
    hash32_t i = 0;

    assert(btree);

    BTREE_FOR_EACH(btree, void, key, value)
        if (i != key)
            break;
        ++i;
    BTREE_END_EACH
    return i;
}

/* ------------------------------------------------------------------------- */
void*
btree_erase(struct btree_t* btree, hash32_t hash)
{
    void* value;
    struct btree_hash_value_t* data;

    assert(btree);

    data = btree_find_lower_bound(btree, hash);
    if (!data || data->hash != hash)
        return NULL;

    value = data->value;
    vector_erase_element(&btree->vector, data);
    return value;
}

/* ------------------------------------------------------------------------- */
void*
btree_erase_element(struct btree_t* btree, void* value)
{
    void* data;
    hash32_t hash;

    assert(btree);

    hash = btree_find_element(btree, value);
    if (hash == BTREE_VECTOR_INVALID_HASH)
        return NULL;

    data = btree_find_lower_bound(btree, hash);
    vector_erase_element(&btree->vector, data);

    return value;
}

/* ------------------------------------------------------------------------- */
void
btree_clear(struct btree_t* btree)
{
    assert(btree);
    vector_clear(&btree->vector);
}

/* ------------------------------------------------------------------------- */
void btree_clear_free(struct btree_t* btree)
{
    assert(btree);
    vector_clear_free(&btree->vector);
}
