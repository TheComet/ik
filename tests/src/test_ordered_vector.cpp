#include "gmock/gmock.h"
#include "ik/ordered_vector.h"

#define NAME ordered_vector

TEST(NAME, init)
{
    struct ordered_vector_t vec;

    vec.capacity = 45;
    vec.count = 384;
    vec.data = (DATA_POINTER_TYPE*)4859;
    vec.element_size = 183;
    ordered_vector_construct(&vec, sizeof(int));

    ASSERT_EQ(0u, vec.capacity);
    ASSERT_EQ(0u, vec.count);
    ASSERT_EQ(NULL, vec.data);
    ASSERT_EQ(sizeof(int), vec.element_size);
}

TEST(NAME, create_initialises_vector)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    ASSERT_EQ(0u, vec->capacity);
    ASSERT_EQ(0u, vec->count);
    ASSERT_EQ(NULL, vec->data);
    ASSERT_EQ(sizeof(int), vec->element_size);
    ordered_vector_destroy(vec);
}

TEST(NAME, push_increments_count_and_causes_realloc_by_factor_2)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    int x = 9;

    ordered_vector_push(vec, &x);
    ASSERT_EQ(2u, vec->capacity);
    ASSERT_EQ(1u, vec->count);

    ordered_vector_push(vec, &x);
    ASSERT_EQ(2u, vec->capacity);
    ASSERT_EQ(2u, vec->count);

    ordered_vector_push(vec, &x);
    ASSERT_EQ(4u, vec->capacity);
    ASSERT_EQ(3u, vec->count);

    ordered_vector_destroy(vec);
}

TEST(NAME, clear_keeps_buffer_and_resets_count)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    int x = 9;
    ordered_vector_push(vec, &x);
    ordered_vector_clear(vec);
    ASSERT_EQ(0u, vec->count);
    ASSERT_EQ(2u, vec->capacity);
    ASSERT_NE((void*)0, vec->data);
    ordered_vector_destroy(vec);
}

TEST(NAME, clear_free_deletes_buffer_and_resets_count)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    int x = 9;
    ordered_vector_push(vec, &x);
    ordered_vector_clear_free(vec);
    ASSERT_EQ(0u, vec->count);
    ASSERT_EQ(0u, vec->capacity);
    ASSERT_EQ(NULL, vec->data);
    ordered_vector_destroy(vec);
}

TEST(NAME, push_emplace_increments_count_and_causes_realloc_by_factor_2)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    ordered_vector_push_emplace(vec);
    ASSERT_EQ(2u, vec->capacity);
    ASSERT_EQ(1u, vec->count);
    ordered_vector_push_emplace(vec);
    ASSERT_EQ(2u, vec->capacity);
    ASSERT_EQ(2u, vec->count);
    ordered_vector_push_emplace(vec);
    ASSERT_EQ(4u, vec->capacity);
    ASSERT_EQ(3u, vec->count);
    ordered_vector_destroy(vec);
}

TEST(NAME, pop_returns_pushed_values)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    int x;
    x = 3; ordered_vector_push(vec, &x);
    x = 2; ordered_vector_push(vec, &x);
    x = 6; ordered_vector_push(vec, &x);
    ASSERT_EQ(6, *(int*)ordered_vector_pop(vec));
    x = 23; ordered_vector_push(vec, &x);
    x = 21; ordered_vector_push(vec, &x);
    ASSERT_EQ(21, *(int*)ordered_vector_pop(vec));
    ASSERT_EQ(23, *(int*)ordered_vector_pop(vec));
    ASSERT_EQ(2, *(int*)ordered_vector_pop(vec));
    ASSERT_EQ(3, *(int*)ordered_vector_pop(vec));
    ASSERT_EQ(0u, vec->count);
    ASSERT_EQ(4u, vec->capacity);
    ASSERT_NE((void*)0, vec->data);
    ordered_vector_destroy(vec);
}

TEST(NAME, pop_returns_push_emplaced_values)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    *(int*)ordered_vector_push_emplace(vec) = 53;
    *(int*)ordered_vector_push_emplace(vec) = 24;
    *(int*)ordered_vector_push_emplace(vec) = 73;
    ASSERT_EQ(73, *(int*)ordered_vector_pop(vec));
    *(int*)ordered_vector_push_emplace(vec) = 28;
    *(int*)ordered_vector_push_emplace(vec) = 72;
    ASSERT_EQ(72, *(int*)ordered_vector_pop(vec));
    ASSERT_EQ(28, *(int*)ordered_vector_pop(vec));
    ASSERT_EQ(24, *(int*)ordered_vector_pop(vec));
    ASSERT_EQ(53, *(int*)ordered_vector_pop(vec));
    ASSERT_EQ(0u, vec->count);
    ASSERT_EQ(4u, vec->capacity);
    ASSERT_NE((void*)0, vec->data);
    ordered_vector_destroy(vec);
}

TEST(NAME, pop_empty_vector)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    *(int*)ordered_vector_push_emplace(vec) = 21;
    ordered_vector_pop(vec);
    ASSERT_EQ(NULL, ordered_vector_pop(vec));
    ASSERT_EQ(0u, vec->count);
    ASSERT_EQ(2u, vec->capacity);
    ASSERT_NE((void*)0, vec->data);
    ordered_vector_destroy(vec);
}

TEST(NAME, pop_clear_freed_vector)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    ASSERT_EQ(NULL, ordered_vector_pop(vec));
    ASSERT_EQ(0u, vec->count);
    ASSERT_EQ(0u, vec->capacity);
    ASSERT_EQ(NULL, vec->data);
    ordered_vector_destroy(vec);
}

TEST(NAME, get_element_random_access)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    *(int*)ordered_vector_push_emplace(vec) = 53;
    *(int*)ordered_vector_push_emplace(vec) = 24;
    *(int*)ordered_vector_push_emplace(vec) = 73;
    *(int*)ordered_vector_push_emplace(vec) = 43;
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 0));
    ordered_vector_destroy(vec);
}

TEST(NAME, popping_preserves_existing_elements)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    *(int*)ordered_vector_push_emplace(vec) = 53;
    *(int*)ordered_vector_push_emplace(vec) = 24;
    *(int*)ordered_vector_push_emplace(vec) = 73;
    *(int*)ordered_vector_push_emplace(vec) = 43;
    *(int*)ordered_vector_push_emplace(vec) = 24;
    ordered_vector_pop(vec);
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 0));
    ordered_vector_destroy(vec);
}

TEST(NAME, erasing_by_index_preserves_existing_elements)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    *(int*)ordered_vector_push_emplace(vec) = 53;
    *(int*)ordered_vector_push_emplace(vec) = 24;
    *(int*)ordered_vector_push_emplace(vec) = 73;
    *(int*)ordered_vector_push_emplace(vec) = 43;
    *(int*)ordered_vector_push_emplace(vec) = 65;
    ordered_vector_erase_index(vec, 1);
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 3));
    ordered_vector_erase_index(vec, 1);
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 2));
    ordered_vector_destroy(vec);
}

TEST(NAME, erasing_by_element_preserves_existing_elements)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    *(int*)ordered_vector_push_emplace(vec) = 53;
    *(int*)ordered_vector_push_emplace(vec) = 24;
    *(int*)ordered_vector_push_emplace(vec) = 73;
    *(int*)ordered_vector_push_emplace(vec) = 43;
    *(int*)ordered_vector_push_emplace(vec) = 65;
    ordered_vector_erase_element(vec, ordered_vector_get_element(vec, 1));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 3));
    ordered_vector_erase_element(vec, ordered_vector_get_element(vec, 1));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 2));
    ordered_vector_destroy(vec);
}

TEST(NAME, get_invalid_index)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    ASSERT_EQ(NULL, ordered_vector_get_element(vec, 1));
    *(int*)ordered_vector_push_emplace(vec) = 53;
    ASSERT_EQ(NULL, ordered_vector_get_element(vec, 1));
    ordered_vector_destroy(vec);
}

TEST(NAME, erase_invalid_index)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    ordered_vector_erase_index(vec, 1);
    ordered_vector_erase_index(vec, 0);
    *(int*)ordered_vector_push_emplace(vec) = 53;
    ordered_vector_erase_index(vec, 1);
    ordered_vector_erase_index(vec, 0);
    ordered_vector_erase_index(vec, 0);
    ordered_vector_destroy(vec);
}

/* ========================================================================= */
/* EVERYTHING ABOVE THIS POINT IS IDENTICAL TO unordered_vector              */
/* ========================================================================= */
/* EVERYTHING BELOW THIS POINT IS UNIQUE TO ordered_vector                   */
/* ========================================================================= */

TEST(NAME, inserting_preserves_existing_elements)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    *(int*)ordered_vector_push_emplace(vec) = 53;
    *(int*)ordered_vector_push_emplace(vec) = 24;
    *(int*)ordered_vector_push_emplace(vec) = 73;
    *(int*)ordered_vector_push_emplace(vec) = 43;
    *(int*)ordered_vector_push_emplace(vec) = 65;

    int x = 68;
    ordered_vector_insert(vec, 2, &x); // middle insertion

    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(68, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 4));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 5));

    x = 16;
    ordered_vector_insert(vec, 0, &x); // beginning insertion

    ASSERT_EQ(16, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(68, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 4));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 5));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 6));

    x = 82;
    ordered_vector_insert(vec, 7, &x); // end insertion

    ASSERT_EQ(16, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(68, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 4));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 5));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 6));
    ASSERT_EQ(82, *(int*)ordered_vector_get_element(vec, 7));

    x = 37;
    ordered_vector_insert(vec, 7, &x); // end insertion

    ASSERT_EQ(16, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(68, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 4));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 5));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 6));
    ASSERT_EQ(37, *(int*)ordered_vector_get_element(vec, 7));
    ASSERT_EQ(82, *(int*)ordered_vector_get_element(vec, 8));

    ordered_vector_destroy(vec);
}

TEST(NAME, insert_emplacing_preserves_existing_elements)
{
    struct ordered_vector_t* vec = ordered_vector_create(sizeof(int));
    *(int*)ordered_vector_push_emplace(vec) = 53;
    *(int*)ordered_vector_push_emplace(vec) = 24;
    *(int*)ordered_vector_push_emplace(vec) = 73;
    *(int*)ordered_vector_push_emplace(vec) = 43;
    *(int*)ordered_vector_push_emplace(vec) = 65;

    *(int*)ordered_vector_insert_emplace(vec, 2) = 68; // middle insertion

    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(68, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 4));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 5));

    *(int*)ordered_vector_insert_emplace(vec, 0) = 16; // beginning insertion

    ASSERT_EQ(16, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(68, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 4));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 5));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 6));

    *(int*)ordered_vector_insert_emplace(vec, 7) = 82; // end insertion

    ASSERT_EQ(16, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(68, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 4));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 5));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 6));
    ASSERT_EQ(82, *(int*)ordered_vector_get_element(vec, 7));

    *(int*)ordered_vector_insert_emplace(vec, 7) = 37; // end insertion

    ASSERT_EQ(16, *(int*)ordered_vector_get_element(vec, 0));
    ASSERT_EQ(53, *(int*)ordered_vector_get_element(vec, 1));
    ASSERT_EQ(24, *(int*)ordered_vector_get_element(vec, 2));
    ASSERT_EQ(68, *(int*)ordered_vector_get_element(vec, 3));
    ASSERT_EQ(73, *(int*)ordered_vector_get_element(vec, 4));
    ASSERT_EQ(43, *(int*)ordered_vector_get_element(vec, 5));
    ASSERT_EQ(65, *(int*)ordered_vector_get_element(vec, 6));
    ASSERT_EQ(37, *(int*)ordered_vector_get_element(vec, 7));
    ASSERT_EQ(82, *(int*)ordered_vector_get_element(vec, 8));

    ordered_vector_destroy(vec);
}
