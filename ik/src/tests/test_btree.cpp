#include "gmock/gmock.h"
#include "ik/btree.h"

#define NAME btree

using namespace testing;

TEST(NAME, init_sets_correct_values)
{
    struct btree_t btree;
    btree.vector.count = 4;
    btree.vector.capacity = 56;
    btree.vector.data = (uint8_t*)4783;
    btree.vector.element_size = 283;

    btree_construct(&btree);
    ASSERT_THAT(btree.vector.count, Eq(0u));

    ASSERT_THAT(btree.vector.capacity, Eq(0u));
    ASSERT_THAT(btree.vector.count, Eq(0u));
    ASSERT_THAT(btree.vector.data, IsNull());
    ASSERT_THAT(btree.vector.element_size, Eq(sizeof(struct btree_hash_value_t )));
}

TEST(NAME, create_initialises_btree)
{
    struct btree_t* btree = btree_create();
    ASSERT_THAT(btree->vector.capacity, Eq(0u));
    ASSERT_THAT(btree->vector.count, Eq(0u));
    ASSERT_THAT(btree->vector.data, IsNull());
    ASSERT_THAT(btree->vector.element_size, Eq(sizeof(struct btree_hash_value_t )));
    btree_destroy(btree);
}

TEST(NAME, insertion_forwards)
{
    struct btree_t* btree = btree_create();

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &b);
    btree_insert(btree, 2, &c);
    btree_insert(btree, 3, &d);
    btree_insert(btree, 4, &e);

    ASSERT_THAT(*(int*)btree_find(btree, 0), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 2), Eq(c));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(d));
    ASSERT_THAT(*(int*)btree_find(btree, 4), Eq(e));

    btree_destroy(btree);
}

TEST(NAME, insertion_backwards)
{
    struct btree_t* btree = btree_create();

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 4, &a);
    btree_insert(btree, 3, &b);
    btree_insert(btree, 2, &c);
    btree_insert(btree, 1, &d);
    btree_insert(btree, 0, &e);

    ASSERT_THAT(*(int*)btree_find(btree, 0), Eq(e));
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(d));
    ASSERT_THAT(*(int*)btree_find(btree, 2), Eq(c));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 4), Eq(a));

    btree_destroy(btree);
}

TEST(NAME, insertion_random)
{
    struct btree_t* btree = btree_create();

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 26, &a);
    btree_insert(btree, 44, &b);
    btree_insert(btree, 82, &c);
    btree_insert(btree, 41, &d);
    btree_insert(btree, 70, &e);

    ASSERT_THAT(*(int*)btree_find(btree, 26), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 41), Eq(d));
    ASSERT_THAT(*(int*)btree_find(btree, 44), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 70), Eq(e));
    ASSERT_THAT(*(int*)btree_find(btree, 82), Eq(c));

    btree_destroy(btree);
}

TEST(NAME, clear_keeps_underlying_vector)
{
    struct btree_t* btree = btree_create();

    int a = 53;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &a);
    btree_insert(btree, 2, &a);

    // this should delete all entries but keep the underlying vector
    btree_clear(btree);

    ASSERT_THAT(btree->vector.count, Eq(0u));
    EXPECT_THAT(btree->vector.data, NotNull());

    btree_destroy(btree);
}

TEST(NAME, clear_free_deletes_underlying_vector)
{
    struct btree_t* btree = btree_create();

    int a=53;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &a);
    btree_insert(btree, 2, &a);

    // this should delete all entries + free the underlying vector
    btree_clear_free(btree);

    ASSERT_THAT(btree->vector.count, Eq(0u));
    ASSERT_THAT(btree->vector.data, IsNull());

    btree_destroy(btree);
}

TEST(NAME, count_returns_correct_number)
{
    struct btree_t* btree = btree_create();

    int a=53;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &a);
    btree_insert(btree, 2, &a);

    ASSERT_THAT(btree_count(btree), Eq(3u));

    btree_destroy(btree);
}

TEST(NAME, erase_elements)
{
    struct btree_t* btree = btree_create();

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &b);
    btree_insert(btree, 2, &c);
    btree_insert(btree, 3, &d);
    btree_insert(btree, 4, &e);

    ASSERT_THAT(*(int*)btree_erase(btree, 2), Eq(c));

    // 4
    ASSERT_THAT(*(int*)btree_find(btree, 0), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(d));
    ASSERT_THAT(*(int*)btree_find(btree, 4), Eq(e));

    ASSERT_THAT(*(int*)btree_erase(btree, 4), Eq(e));

    // 3
    ASSERT_THAT(*(int*)btree_find(btree, 0), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(d));

    ASSERT_THAT(*(int*)btree_erase(btree, 0), Eq(a));

    // 2
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(d));

    ASSERT_THAT(*(int*)btree_erase(btree, 1), Eq(b));

    // 1
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(d));

    ASSERT_THAT(*(int*)btree_erase(btree, 3), Eq(d));

    ASSERT_THAT(btree_erase(btree, 2), IsNull());

    btree_destroy(btree);
}

TEST(NAME, reinsertion_forwards)
{
    struct btree_t* btree = btree_create();

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 0, &a);
    btree_insert(btree, 1, &b);
    btree_insert(btree, 2, &c);
    btree_insert(btree, 3, &d);
    btree_insert(btree, 4, &e);

    btree_erase(btree, 4);
    btree_erase(btree, 3);
    btree_erase(btree, 2);

    btree_insert(btree, 2, &c);
    btree_insert(btree, 3, &d);
    btree_insert(btree, 4, &e);

    ASSERT_THAT( *(int*)btree_find(btree, 0), Eq(a));
    ASSERT_THAT( *(int*)btree_find(btree, 1), Eq(b));
    ASSERT_THAT( *(int*)btree_find(btree, 2), Eq(c));
    ASSERT_THAT( *(int*)btree_find(btree, 3), Eq(d));
    ASSERT_THAT( *(int*)btree_find(btree, 4), Eq(e));

    btree_destroy(btree);
}

TEST(NAME, reinsertion_backwards)
{
    struct btree_t* btree = btree_create();

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 4, &a);
    btree_insert(btree, 3, &b);
    btree_insert(btree, 2, &c);
    btree_insert(btree, 1, &d);
    btree_insert(btree, 0, &e);

    btree_erase(btree, 0);
    btree_erase(btree, 1);
    btree_erase(btree, 2);

    btree_insert(btree, 2, &c);
    btree_insert(btree, 1, &d);
    btree_insert(btree, 0, &e);

    ASSERT_THAT(*(int*)btree_find(btree, 0), Eq(e));
    ASSERT_THAT(*(int*)btree_find(btree, 1), Eq(d));
    ASSERT_THAT(*(int*)btree_find(btree, 2), Eq(c));
    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 4), Eq(a));

    btree_destroy(btree);
}

TEST(NAME, reinsertion_random)
{
    struct btree_t* btree = btree_create();

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert(btree, 26, &a);
    btree_insert(btree, 44, &b);
    btree_insert(btree, 82, &c);
    btree_insert(btree, 41, &d);
    btree_insert(btree, 70, &e);

    btree_erase(btree, 44);
    btree_erase(btree, 70);
    btree_erase(btree, 26);

    btree_insert(btree, 26, &a);
    btree_insert(btree, 70, &e);
    btree_insert(btree, 44, &b);

    ASSERT_THAT(*(int*)btree_find(btree, 26), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 41), Eq(d));
    ASSERT_THAT(*(int*)btree_find(btree, 44), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 70), Eq(e));
    ASSERT_THAT(*(int*)btree_find(btree, 82), Eq(c));

    btree_destroy(btree);
}

TEST(NAME, inserting_duplicate_hashes_doesnt_replace_existing_elements)
{
    struct btree_t* btree = btree_create();

    int a=56, b=45, c=18;
    btree_insert(btree, 5, &a);
    btree_insert(btree, 3, &a);

    btree_insert(btree, 5, &b);
    btree_insert(btree, 4, &b);
    btree_insert(btree, 3, &c);

    ASSERT_THAT(*(int*)btree_find(btree, 3), Eq(a));
    ASSERT_THAT(*(int*)btree_find(btree, 4), Eq(b));
    ASSERT_THAT(*(int*)btree_find(btree, 5), Eq(a));

    btree_destroy(btree);
}

TEST(NAME, generating_hashes_do_not_conflict_with_existing_ascending_hashes)
{
    intptr_t hash;
    struct btree_t* btree = btree_create();
    btree_insert(btree, 0, NULL);
    btree_insert(btree, 1, NULL);
    btree_insert(btree, 2, NULL);
    btree_insert(btree, 3, NULL);
    btree_insert(btree, 5, NULL);
    hash = btree_find_unused_hash(btree);
    ASSERT_NE(0, hash);
    ASSERT_NE(1, hash);
    ASSERT_NE(2, hash);
    ASSERT_NE(3, hash);
    ASSERT_NE(5, hash);
    btree_destroy(btree);
}

TEST(NAME, generating_hashes_do_not_conflict_with_existing_descending_hashes)
{
    intptr_t hash;
    struct btree_t* btree = btree_create();
    btree_insert(btree, 5, NULL);
    btree_insert(btree, 3, NULL);
    btree_insert(btree, 2, NULL);
    btree_insert(btree, 1, NULL);
    btree_insert(btree, 0, NULL);
    hash = btree_find_unused_hash(btree);
    ASSERT_NE(0, hash);
    ASSERT_NE(1, hash);
    ASSERT_NE(2, hash);
    ASSERT_NE(3, hash);
    ASSERT_NE(5, hash);
    btree_destroy(btree);
}

TEST(NAME, generating_hashes_do_not_conflict_with_existing_random_hashes)
{
    intptr_t hash;
    struct btree_t* btree = btree_create();
    btree_insert(btree, 2387, NULL);
    btree_insert(btree, 28, NULL);
    btree_insert(btree, 358, NULL);
    btree_insert(btree, 183, NULL);
    btree_insert(btree, 38, NULL);
    hash = btree_find_unused_hash(btree);
    ASSERT_NE(2387, hash);
    ASSERT_NE(28, hash);
    ASSERT_NE(358, hash);
    ASSERT_NE(183, hash);
    ASSERT_NE(38, hash);
    btree_destroy(btree);
}

TEST(NAME, find_element)
{
    struct btree_t* btree = btree_create();
    int a = 6;
    btree_insert(btree, 2387, NULL);
    btree_insert(btree, 28, &a);
    btree_insert(btree, 358, NULL);
    btree_insert(btree, 183, NULL);
    btree_insert(btree, 38, NULL);

    EXPECT_THAT(btree_find_element(btree, &a), Eq(28u));

    btree_destroy(btree);
}

TEST(NAME, set_value)
{
    struct btree_t* btree = btree_create();
    int a = 6;
    btree_insert(btree, 2387, NULL);
    btree_insert(btree, 28, NULL);
    btree_insert(btree, 358, NULL);
    btree_insert(btree, 183, NULL);
    btree_insert(btree, 38, NULL);

    btree_set(btree, 28, &a);

    EXPECT_THAT((int*)btree_find(btree, 28), Pointee(a));

    btree_destroy(btree);
}

TEST(NAME, get_any_element)
{
    struct btree_t* btree = btree_create();
    int a = 6;

    EXPECT_THAT(btree_get_any_element(btree), IsNull());
    btree_insert(btree, 45, &a);
    EXPECT_THAT(btree_get_any_element(btree), NotNull());
    btree_erase(btree, 45);
    EXPECT_THAT(btree_get_any_element(btree), IsNull());

    btree_destroy(btree);
}

TEST(NAME, hash_exists)
{
    struct btree_t* btree = btree_create();

    EXPECT_THAT(btree_hash_exists(btree, 29), Eq(-1));
    btree_insert(btree, 29, NULL);
    EXPECT_THAT(btree_hash_exists(btree, 29), Eq(0));
    EXPECT_THAT(btree_hash_exists(btree, 40), Eq(-1));
    btree_erase(btree, 29);
    EXPECT_THAT(btree_hash_exists(btree, 29), Eq(-1));

    btree_destroy(btree);
}

TEST(NAME, erase_element)
{
    struct btree_t* btree = btree_create();
    int a = 6;

    EXPECT_THAT(btree_erase_element(btree, &a), IsNull());
    EXPECT_THAT(btree_erase_element(btree, NULL), IsNull());
    btree_insert(btree, 39, &a);
    EXPECT_THAT((int*)btree_erase_element(btree, &a), Pointee(a));

    btree_destroy(btree);
}

TEST(NAME, iterate_with_no_items)
{
    struct btree_t* btree = btree_create();
    {
        int counter = 0;
        BTREE_FOR_EACH(btree, int, hash, value)
            ++counter;
        BTREE_END_EACH
        ASSERT_THAT(counter, Eq(0));
    }
    btree_destroy(btree);
}

TEST(NAME, iterate_5_random_items)
{
    struct btree_t* btree = btree_create();

    int a=79579, b=235, c=347, d=124, e=457;
    btree_insert(btree, 243, &a);
    btree_insert(btree, 256, &b);
    btree_insert(btree, 456, &c);
    btree_insert(btree, 468, &d);
    btree_insert(btree, 969, &e);

    int counter = 0;
    BTREE_FOR_EACH(btree, int, hash, value)
        switch(counter)
        {
            case 0 : ASSERT_THAT(hash, Eq(243u)); ASSERT_THAT(a, Eq(*value)); break;
            case 1 : ASSERT_THAT(hash, Eq(256u)); ASSERT_THAT(b, Eq(*value)); break;
            case 2 : ASSERT_THAT(hash, Eq(456u)); ASSERT_THAT(c, Eq(*value)); break;
            case 3 : ASSERT_THAT(hash, Eq(468u)); ASSERT_THAT(d, Eq(*value)); break;
            case 4 : ASSERT_THAT(hash, Eq(969u)); ASSERT_THAT(e, Eq(*value)); break;
            default: ASSERT_THAT(1, Eq(0)); break;
        }
        ++counter;
    BTREE_END_EACH
    ASSERT_THAT(counter, Eq(5));

    btree_destroy(btree);
}

TEST(NAME, iterate_5_null_items)
{
    struct btree_t* btree = btree_create();

    btree_insert(btree, 243, NULL);
    btree_insert(btree, 256, NULL);
    btree_insert(btree, 456, NULL);
    btree_insert(btree, 468, NULL);
    btree_insert(btree, 969, NULL);

    int counter = 0;
    BTREE_FOR_EACH(btree, int, hash, value)
        switch(counter)
        {
            case 0 : ASSERT_THAT(hash, Eq(243u)); ASSERT_THAT(value, IsNull()); break;
            case 1 : ASSERT_THAT(hash, Eq(256u)); ASSERT_THAT(value, IsNull()); break;
            case 2 : ASSERT_THAT(hash, Eq(456u)); ASSERT_THAT(value, IsNull()); break;
            case 3 : ASSERT_THAT(hash, Eq(468u)); ASSERT_THAT(value, IsNull()); break;
            case 4 : ASSERT_THAT(hash, Eq(969u)); ASSERT_THAT(value, IsNull()); break;
            default: ASSERT_THAT(1, Eq(0)); break;
        }
        ++counter;
    BTREE_END_EACH
    ASSERT_THAT(counter, Eq(5));

    btree_destroy(btree);
}

TEST(NAME, erase_in_for_loop)
{
    struct btree_t* btree = btree_create();

    int a=79579, b=235, c=347, d=124, e=457;
    btree_insert(btree, 243, &a);
    btree_insert(btree, 256, &b);
    btree_insert(btree, 456, &c);
    btree_insert(btree, 468, &d);
    btree_insert(btree, 969, &e);

    BTREE_FOR_EACH(btree, int, hash, value)
        if(hash == 256u)
            BTREE_ERASE_CURRENT_ITEM_IN_FOR_LOOP(btree, value);
    BTREE_END_EACH

    EXPECT_THAT((int*)btree_find(btree, 243), Pointee(a));
    EXPECT_THAT((int*)btree_find(btree, 256), IsNull());
    EXPECT_THAT((int*)btree_find(btree, 456), Pointee(c));
    EXPECT_THAT((int*)btree_find(btree, 468), Pointee(d));
    EXPECT_THAT((int*)btree_find(btree, 969), Pointee(e));

    btree_destroy(btree);
}
