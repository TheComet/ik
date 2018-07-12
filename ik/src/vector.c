#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "ik/vector.h"
#include "ik/memory.h"

/* ----------------------------------------------------------------------------
 * Static functions
 * ------------------------------------------------------------------------- */

/*!
 * @brief Expands the underlying memory.
 *
 * This implementation will expand the memory by a factor of 2 each time this
 * is called. All elements are copied into the new section of memory.
 * @param[in] insertion_index Set to -1 if (no space should be made for element
 * insertion. Otherwise this parameter specifies the index of the element to
 * "evade" when re-allocating all other elements.
 * @param[in] target_size If set to 0, target size is calculated automatically.
 * Otherwise the vector will expand to the specified target size.
 * @note No checks are performed to make sure the target size is large enough.
 */
static ikret_t
vector_expand(struct vector_t *vector,
              vector_size_t insertion_index,
              vector_size_t target_size);

/* ----------------------------------------------------------------------------
 * Exported functions
 * ------------------------------------------------------------------------- */
struct vector_t*
vector_create(const uint32_t element_size)
{
    struct vector_t* vector;
    if (!(vector = MALLOC(sizeof *vector)))
        return NULL;
    vector_construct(vector, element_size);
    return vector;
}

/* ------------------------------------------------------------------------- */
void
vector_construct(struct vector_t* vector, const uint32_t element_size)
{
    assert(vector);
    memset(vector, 0, sizeof *vector);
    vector->element_size = element_size;
}

/* ------------------------------------------------------------------------- */
void
vector_destroy(struct vector_t* vector)
{
    assert(vector);
    vector_clear_free(vector);
    FREE(vector);
}

/* ------------------------------------------------------------------------- */
void
vector_clear(struct vector_t* vector)
{
    assert(vector);
    /*
     * No need to free or overwrite existing memory, just reset the counter
     * and let future insertions overwrite
     */
    vector->count = 0;
}

/* ------------------------------------------------------------------------- */
void
vector_clear_free(struct vector_t* vector)
{
    assert(vector);

    if (vector->data)
        FREE(vector->data);

    vector->data = NULL;
    vector->count = 0;
    vector->capacity = 0;
}

/* ------------------------------------------------------------------------- */
ikret_t
vector_resize(struct vector_t* vector, uint32_t size)
{
    assert(vector);

    if (vector->capacity < size)
    {
        ikret_t result;
        if ((result = vector_expand(vector, VECTOR_ERROR, size)) != IK_OK)
            return result;
    }

    vector->count = size;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void*
vector_push_emplace(struct vector_t* vector)
{
    void* data;

    assert(vector);

    if (vector->count == vector->capacity)
        if (vector_expand(vector, VECTOR_ERROR, 0) != IK_OK)
            return NULL;
    data = vector->data + (vector->element_size * vector->count);
    ++(vector->count);
    return data;
}

/* ------------------------------------------------------------------------- */
ikret_t
vector_push(struct vector_t* vector, const void* data)
{
    void* emplaced;

    assert(vector);
    assert(data);

    emplaced = vector_push_emplace(vector);
    if (!emplaced)
        return IK_RAN_OUT_OF_MEMORY;
    memcpy(emplaced, data, vector->element_size);
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
ikret_t
vector_push_vector(struct vector_t* vector, struct vector_t* source_vector)
{
    ikret_t result;

    assert(vector);
    assert(source_vector);

    /* make sure element sizes are equal */
    if (vector->element_size != source_vector->element_size)
        return IK_VECTOR_HAS_DIFFERENT_ELEMENT_SIZE;

    /* make sure there's enough space in the target vector */
    if (vector->count + source_vector->count > vector->capacity)
        if ((result = vector_expand(vector, VECTOR_ERROR, vector->count + source_vector->count)) != IK_OK)
            return result;

    /* copy data */
    memcpy(vector->data + (vector->count * vector->element_size),
           source_vector->data,
           source_vector->count * vector->element_size);
    vector->count += source_vector->count;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void*
vector_pop(struct vector_t* vector)
{
    assert(vector);

    if (!vector->count)
        return NULL;

    --(vector->count);
    return vector->data + (vector->element_size * vector->count);
}

/* ------------------------------------------------------------------------- */
void*
vector_back(const struct vector_t* vector)
{
    assert(vector);

    if (!vector->count)
        return NULL;

    return vector->data + (vector->element_size * (vector->count - 1));
}

/* ------------------------------------------------------------------------- */
void*
vector_insert_emplace(struct vector_t* vector, uint32_t index)
{
    uint32_t offset;

    assert(vector);

    /*
     * Normally the last valid index is (capacity-1), but in this case it's valid
     * because it's possible the user will want to insert at the very end of
     * the vector.
     */
    if (index > vector->count)
        return NULL;

    /* re-allocate? */
    if (vector->count == vector->capacity)
    {
        if (vector_expand(vector, index, 0) < 0)
            return NULL;
    }
    else
    {
        /* shift all elements up by one to make space for insertion */
        uint32_t total_size = vector->count * vector->element_size;
        offset = vector->element_size * index;
        memmove((void*)((intptr_t)vector->data + offset + vector->element_size),
                (void*)((intptr_t)vector->data + offset),
                total_size - offset);
    }

    /* return pointer to memory of new element */
    ++vector->count;
    return (void*)(vector->data + index * vector->element_size);
}

/* ------------------------------------------------------------------------- */
ikret_t
vector_insert(struct vector_t* vector, uint32_t index, void* data)
{
    void* emplaced;

    assert(vector);
    assert(data);

    emplaced = vector_insert_emplace(vector, index);
    if (!emplaced)
        return IK_RAN_OUT_OF_MEMORY;
    memcpy(emplaced, data, vector->element_size);
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
vector_erase_index(struct vector_t* vector, uint32_t index)
{
    assert(vector);

    if (index >= vector->count)
        return;

    if (index == vector->count - 1)
        /* last element doesn't require memory shifting, just pop it */
        vector_pop(vector);
    else
    {
        /* shift memory right after the specified element down by one element */
        uint32_t offset = vector->element_size * index;  /* offset to the element being erased in bytes */
        uint32_t total_size = vector->element_size * vector->count; /* total current size in bytes */
        memmove((void*)((intptr_t)vector->data + offset),   /* target is to overwrite the element specified by index */
                (void*)((intptr_t)vector->data + offset + vector->element_size),    /* copy beginning from one element ahead of element to be erased */
                total_size - offset - vector->element_size);     /* copying number of elements after element to be erased */
        --vector->count;
    }
}

/* ------------------------------------------------------------------------- */
void
vector_erase_element(struct vector_t* vector, void* element)
{
    uintptr_t last_element;

    assert(vector);
    last_element = (uintptr_t)vector->data + (vector->count-1) * vector->element_size;
    assert(element);
    assert((uintptr_t)element >= (uintptr_t)vector->data);
    assert((uintptr_t)element <= (uintptr_t)last_element);

    if (element != (void*)last_element)
    {
        memmove(element,    /* target is to overwrite the element */
                (void*)((uintptr_t)element + vector->element_size), /* read everything from next element */
                last_element - (uintptr_t)element);
    }
    --vector->count;
}

/* ------------------------------------------------------------------------- */
void*
vector_get_element(const struct vector_t* vector, uint32_t index)
{
    assert(vector);

    if (index >= vector->count)
        return NULL;
    return vector->data + (vector->element_size * index);
}

/* ----------------------------------------------------------------------------
 * Static functions
 * ------------------------------------------------------------------------- */
static ikret_t
vector_expand(struct vector_t *vector,
              vector_size_t insertion_index,
              vector_size_t target_count)
{
    vector_size_t new_count;
    uint8_t* old_data;
    uint8_t* new_data;

    /* expand by factor 2, or adopt target count if (it is not 0 */
    if (target_count)
        new_count = target_count;
    else
        new_count = vector->capacity << 1;

    /*
     * If vector hasn't allocated anything yet, just allocated the requested
     * amount of memory and return immediately.
     */
    if (!vector->data)
    {
        new_count = (new_count == 0 ? 2 : new_count);
        vector->data = MALLOC(new_count * vector->element_size);
        if (!vector->data)
            return IK_RAN_OUT_OF_MEMORY;
        vector->capacity = new_count;
        return IK_OK;
    }

    /* prepare for reallocating data */
    old_data = vector->data;
    new_data = MALLOC(new_count * vector->element_size);
    if (!new_data)
        return IK_RAN_OUT_OF_MEMORY;

    /* if (no insertion index is required, copy all data to new memory */
    if (insertion_index == VECTOR_ERROR || insertion_index >= new_count)
        memcpy(new_data, old_data, vector->count * vector->element_size);

    /* keep space for one element at the insertion index */
    else
    {
        /* copy old data up until right before insertion offset */
        vector_size_t offset = vector->element_size * insertion_index;
        vector_size_t total_size = vector->element_size * vector->count;
        memcpy(new_data, old_data, offset);
        /* copy the remaining amount of old data shifted one element ahead */
        memcpy((void*)((uintptr_t)new_data + offset + vector->element_size),
               (void*)((uintptr_t)old_data + offset),
               total_size - offset);
    }

    vector->data = new_data;
    vector->capacity = new_count;
    FREE(old_data);

    return IK_OK;
}
