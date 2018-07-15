#include "gmock/gmock.h"
#include "ik/bstv.h"

#define NAME bstv

using namespace testing;

TEST(NAME, init_sets_correct_values)
{
    struct bstv_t bstv;
    bstv.vector.count = 4;
    bstv.vector.capacity = 56;
    bstv.vector.data = (uint8_t*)4783;
    bstv.vector.element_size = 283;

    bstv_construct(&bstv);
    ASSERT_EQ(0u, bstv.vector.count);

    ASSERT_EQ(0u, bstv.vector.capacity);
    ASSERT_EQ(0u, bstv.vector.count);
    ASSERT_EQ(NULL, bstv.vector.data);
    ASSERT_EQ(sizeof(struct bstv_hash_value_t), bstv.vector.element_size);
}

TEST(NAME, create_initialises_bstv)
{
    struct bstv_t* bstv = bstv_create();
    ASSERT_EQ(0u, bstv->vector.capacity);
    ASSERT_EQ(0u, bstv->vector.count);
    ASSERT_EQ(NULL, bstv->vector.data);
    ASSERT_EQ(sizeof(struct bstv_hash_value_t), bstv->vector.element_size);
    bstv_destroy(bstv);
}

TEST(NAME, insertion_forwards)
{
    struct bstv_t* bstv = bstv_create();

    int a=56, b=45, c=18, d=27, e=84;
    bstv_insert(bstv, 0, &a);
    bstv_insert(bstv, 1, &b);
    bstv_insert(bstv, 2, &c);
    bstv_insert(bstv, 3, &d);
    bstv_insert(bstv, 4, &e);

    ASSERT_EQ(a, *(int*)bstv_find(bstv, 0));
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 1));
    ASSERT_EQ(c, *(int*)bstv_find(bstv, 2));
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 3));
    ASSERT_EQ(e, *(int*)bstv_find(bstv, 4));

    bstv_destroy(bstv);
}

TEST(NAME, insertion_backwards)
{
    struct bstv_t* bstv = bstv_create();

    int a=56, b=45, c=18, d=27, e=84;
    bstv_insert(bstv, 4, &a);
    bstv_insert(bstv, 3, &b);
    bstv_insert(bstv, 2, &c);
    bstv_insert(bstv, 1, &d);
    bstv_insert(bstv, 0, &e);

    ASSERT_EQ(e, *(int*)bstv_find(bstv, 0));
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 1));
    ASSERT_EQ(c, *(int*)bstv_find(bstv, 2));
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 3));
    ASSERT_EQ(a, *(int*)bstv_find(bstv, 4));

    bstv_destroy(bstv);
}

TEST(NAME, insertion_random)
{
    struct bstv_t* bstv = bstv_create();

    int a=56, b=45, c=18, d=27, e=84;
    bstv_insert(bstv, 26, &a);
    bstv_insert(bstv, 44, &b);
    bstv_insert(bstv, 82, &c);
    bstv_insert(bstv, 41, &d);
    bstv_insert(bstv, 70, &e);

    ASSERT_EQ(a, *(int*)bstv_find(bstv, 26));
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 41));
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 44));
    ASSERT_EQ(e, *(int*)bstv_find(bstv, 70));
    ASSERT_EQ(c, *(int*)bstv_find(bstv, 82));

    bstv_destroy(bstv);
}

TEST(NAME, clear_keeps_underlying_vector)
{
    struct bstv_t* bstv = bstv_create();

    int a = 53;
    bstv_insert(bstv, 0, &a);
    bstv_insert(bstv, 1, &a);
    bstv_insert(bstv, 2, &a);

    // this should delete all entries but keep the underlying vector
    bstv_clear(bstv);

    ASSERT_EQ(0u, bstv->vector.count);
    EXPECT_THAT(bstv->vector.data, NotNull());

    bstv_destroy(bstv);
}

TEST(NAME, clear_free_deletes_underlying_vector)
{
    struct bstv_t* bstv = bstv_create();

    int a=53;
    bstv_insert(bstv, 0, &a);
    bstv_insert(bstv, 1, &a);
    bstv_insert(bstv, 2, &a);

    // this should delete all entries + free the underlying vector
    bstv_clear_free(bstv);

    ASSERT_EQ(0u, bstv->vector.count);
    ASSERT_EQ(NULL, bstv->vector.data);

    bstv_destroy(bstv);
}

TEST(NAME, count_returns_correct_number)
{
    struct bstv_t* bstv = bstv_create();

    int a=53;
    bstv_insert(bstv, 0, &a);
    bstv_insert(bstv, 1, &a);
    bstv_insert(bstv, 2, &a);

    ASSERT_EQ(3u, bstv_count(bstv));

    bstv_destroy(bstv);
}

TEST(NAME, erase_elements)
{
    struct bstv_t* bstv = bstv_create();

    int a=56, b=45, c=18, d=27, e=84;
    bstv_insert(bstv, 0, &a);
    bstv_insert(bstv, 1, &b);
    bstv_insert(bstv, 2, &c);
    bstv_insert(bstv, 3, &d);
    bstv_insert(bstv, 4, &e);

    ASSERT_EQ(c, *(int*)bstv_erase(bstv, 2));

    // 4
    ASSERT_EQ(a, *(int*)bstv_find(bstv, 0));
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 1));
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 3));
    ASSERT_EQ(e, *(int*)bstv_find(bstv, 4));

    ASSERT_EQ(e, *(int*)bstv_erase(bstv, 4));

    // 3
    ASSERT_EQ(a, *(int*)bstv_find(bstv, 0));
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 1));
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 3));

    ASSERT_EQ(a, *(int*)bstv_erase(bstv, 0));

    // 2
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 1));
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 3));

    ASSERT_EQ(b, *(int*)bstv_erase(bstv, 1));

    // 1
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 3));

    ASSERT_EQ(d, *(int*)bstv_erase(bstv, 3));

    ASSERT_EQ(NULL, bstv_erase(bstv, 2));

    bstv_destroy(bstv);
}

TEST(NAME, reinsertion_forwards)
{
    struct bstv_t* bstv = bstv_create();

    int a=56, b=45, c=18, d=27, e=84;
    bstv_insert(bstv, 0, &a);
    bstv_insert(bstv, 1, &b);
    bstv_insert(bstv, 2, &c);
    bstv_insert(bstv, 3, &d);
    bstv_insert(bstv, 4, &e);

    bstv_erase(bstv, 4);
    bstv_erase(bstv, 3);
    bstv_erase(bstv, 2);

    bstv_insert(bstv, 2, &c);
    bstv_insert(bstv, 3, &d);
    bstv_insert(bstv, 4, &e);

    ASSERT_EQ(a, *(int*)bstv_find(bstv, 0));
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 1));
    ASSERT_EQ(c, *(int*)bstv_find(bstv, 2));
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 3));
    ASSERT_EQ(e, *(int*)bstv_find(bstv, 4));

    bstv_destroy(bstv);
}

TEST(NAME, reinsertion_backwards)
{
    struct bstv_t* bstv = bstv_create();

    int a=56, b=45, c=18, d=27, e=84;
    bstv_insert(bstv, 4, &a);
    bstv_insert(bstv, 3, &b);
    bstv_insert(bstv, 2, &c);
    bstv_insert(bstv, 1, &d);
    bstv_insert(bstv, 0, &e);

    bstv_erase(bstv, 0);
    bstv_erase(bstv, 1);
    bstv_erase(bstv, 2);

    bstv_insert(bstv, 2, &c);
    bstv_insert(bstv, 1, &d);
    bstv_insert(bstv, 0, &e);

    ASSERT_EQ(e, *(int*)bstv_find(bstv, 0));
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 1));
    ASSERT_EQ(c, *(int*)bstv_find(bstv, 2));
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 3));
    ASSERT_EQ(a, *(int*)bstv_find(bstv, 4));

    bstv_destroy(bstv);
}

TEST(NAME, reinsertion_random)
{
    struct bstv_t* bstv = bstv_create();

    int a=56, b=45, c=18, d=27, e=84;
    bstv_insert(bstv, 26, &a);
    bstv_insert(bstv, 44, &b);
    bstv_insert(bstv, 82, &c);
    bstv_insert(bstv, 41, &d);
    bstv_insert(bstv, 70, &e);

    bstv_erase(bstv, 44);
    bstv_erase(bstv, 70);
    bstv_erase(bstv, 26);

    bstv_insert(bstv, 26, &a);
    bstv_insert(bstv, 70, &e);
    bstv_insert(bstv, 44, &b);

    ASSERT_EQ(a, *(int*)bstv_find(bstv, 26));
    ASSERT_EQ(d, *(int*)bstv_find(bstv, 41));
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 44));
    ASSERT_EQ(e, *(int*)bstv_find(bstv, 70));
    ASSERT_EQ(c, *(int*)bstv_find(bstv, 82));

    bstv_destroy(bstv);
}

TEST(NAME, inserting_duplicate_hashes_doesnt_replace_existing_elements)
{
    struct bstv_t* bstv = bstv_create();

    int a=56, b=45, c=18;
    bstv_insert(bstv, 5, &a);
    bstv_insert(bstv, 3, &a);

    bstv_insert(bstv, 5, &b);
    bstv_insert(bstv, 4, &b);
    bstv_insert(bstv, 3, &c);

    ASSERT_EQ(a, *(int*)bstv_find(bstv, 3));
    ASSERT_EQ(b, *(int*)bstv_find(bstv, 4));
    ASSERT_EQ(a, *(int*)bstv_find(bstv, 5));

    bstv_destroy(bstv);
}

TEST(NAME, generating_hashes_do_not_conflict_with_existing_ascending_hashes)
{
    intptr_t hash;
    struct bstv_t* bstv = bstv_create();
    bstv_insert(bstv, 0, NULL);
    bstv_insert(bstv, 1, NULL);
    bstv_insert(bstv, 2, NULL);
    bstv_insert(bstv, 3, NULL);
    bstv_insert(bstv, 5, NULL);
    hash = bstv_find_unused_hash(bstv);
    ASSERT_NE(0, hash);
    ASSERT_NE(1, hash);
    ASSERT_NE(2, hash);
    ASSERT_NE(3, hash);
    ASSERT_NE(5, hash);
    bstv_destroy(bstv);
}

TEST(NAME, generating_hashes_do_not_conflict_with_existing_descending_hashes)
{
    intptr_t hash;
    struct bstv_t* bstv = bstv_create();
    bstv_insert(bstv, 5, NULL);
    bstv_insert(bstv, 3, NULL);
    bstv_insert(bstv, 2, NULL);
    bstv_insert(bstv, 1, NULL);
    bstv_insert(bstv, 0, NULL);
    hash = bstv_find_unused_hash(bstv);
    ASSERT_NE(0, hash);
    ASSERT_NE(1, hash);
    ASSERT_NE(2, hash);
    ASSERT_NE(3, hash);
    ASSERT_NE(5, hash);
    bstv_destroy(bstv);
}

TEST(NAME, generating_hashes_do_not_conflict_with_existing_random_hashes)
{
    intptr_t hash;
    struct bstv_t* bstv = bstv_create();
    bstv_insert(bstv, 2387, NULL);
    bstv_insert(bstv, 28, NULL);
    bstv_insert(bstv, 358, NULL);
    bstv_insert(bstv, 183, NULL);
    bstv_insert(bstv, 38, NULL);
    hash = bstv_find_unused_hash(bstv);
    ASSERT_NE(2387, hash);
    ASSERT_NE(28, hash);
    ASSERT_NE(358, hash);
    ASSERT_NE(183, hash);
    ASSERT_NE(38, hash);
    bstv_destroy(bstv);
}

TEST(NAME, find_element)
{
    struct bstv_t* bstv = bstv_create();
    int a = 6;
    bstv_insert(bstv, 2387, NULL);
    bstv_insert(bstv, 28, &a);
    bstv_insert(bstv, 358, NULL);
    bstv_insert(bstv, 183, NULL);
    bstv_insert(bstv, 38, NULL);

    EXPECT_THAT(bstv_find_element(bstv, &a), Eq(28u));

    bstv_destroy(bstv);
}

TEST(NAME, set_value)
{
    struct bstv_t* bstv = bstv_create();
    int a = 6;
    bstv_insert(bstv, 2387, NULL);
    bstv_insert(bstv, 28, NULL);
    bstv_insert(bstv, 358, NULL);
    bstv_insert(bstv, 183, NULL);
    bstv_insert(bstv, 38, NULL);

    bstv_set(bstv, 28, &a);

    EXPECT_THAT((int*)bstv_find(bstv, 28), Pointee(a));

    bstv_destroy(bstv);
}

TEST(NAME, get_any_element)
{
    struct bstv_t* bstv = bstv_create();
    int a = 6;

    EXPECT_THAT(bstv_get_any_element(bstv), IsNull());
    bstv_insert(bstv, 45, &a);
    EXPECT_THAT(bstv_get_any_element(bstv), NotNull());
    bstv_erase(bstv, 45);
    EXPECT_THAT(bstv_get_any_element(bstv), IsNull());

    bstv_destroy(bstv);
}

TEST(NAME, hash_exists)
{
    struct bstv_t* bstv = bstv_create();

    EXPECT_THAT(bstv_hash_exists(bstv, 29), Eq(IK_HASH_NOT_FOUND));
    bstv_insert(bstv, 29, NULL);
    EXPECT_THAT(bstv_hash_exists(bstv, 29), Eq(IK_OK));
    EXPECT_THAT(bstv_hash_exists(bstv, 40), Eq(IK_HASH_NOT_FOUND));
    bstv_erase(bstv, 29);
    EXPECT_THAT(bstv_hash_exists(bstv, 29), Eq(IK_HASH_NOT_FOUND));

    bstv_destroy(bstv);
}

TEST(NAME, erase_element)
{
    struct bstv_t* bstv = bstv_create();
    int a = 6;

    EXPECT_THAT(bstv_erase_element(bstv, &a), IsNull());
    EXPECT_THAT(bstv_erase_element(bstv, NULL), IsNull());
    bstv_insert(bstv, 39, &a);
    EXPECT_THAT((int*)bstv_erase_element(bstv, &a), Pointee(a));

    bstv_destroy(bstv);
}

TEST(NAME, iterate_with_no_items)
{
    struct bstv_t* bstv = bstv_create();
    {
        int counter = 0;
        BSTV_FOR_EACH(bstv, int, hash, value)
            ++counter;
        BSTV_END_EACH
        ASSERT_EQ(0, counter);
    }
    bstv_destroy(bstv);
}

TEST(NAME, iterate_5_random_items)
{
    struct bstv_t* bstv = bstv_create();

    int a=79579, b=235, c=347, d=124, e=457;
    bstv_insert(bstv, 243, &a);
    bstv_insert(bstv, 256, &b);
    bstv_insert(bstv, 456, &c);
    bstv_insert(bstv, 468, &d);
    bstv_insert(bstv, 969, &e);

    int counter = 0;
    BSTV_FOR_EACH(bstv, int, hash, value)
        switch(counter)
        {
            case 0 : ASSERT_EQ(243u, hash); ASSERT_EQ(a, *value); break;
            case 1 : ASSERT_EQ(256u, hash); ASSERT_EQ(b, *value); break;
            case 2 : ASSERT_EQ(456u, hash); ASSERT_EQ(c, *value); break;
            case 3 : ASSERT_EQ(468u, hash); ASSERT_EQ(d, *value); break;
            case 4 : ASSERT_EQ(969u, hash); ASSERT_EQ(e, *value); break;
            default: ASSERT_EQ(0, 1); break;
        }
        ++counter;
    BSTV_END_EACH
    ASSERT_EQ(5, counter);

    bstv_destroy(bstv);
}

TEST(NAME, iterate_5_null_items)
{
    struct bstv_t* bstv = bstv_create();

    bstv_insert(bstv, 243, NULL);
    bstv_insert(bstv, 256, NULL);
    bstv_insert(bstv, 456, NULL);
    bstv_insert(bstv, 468, NULL);
    bstv_insert(bstv, 969, NULL);

    int counter = 0;
    BSTV_FOR_EACH(bstv, int, hash, value)
        switch(counter)
        {
            case 0 : ASSERT_EQ(243u, hash); ASSERT_EQ(NULL, value); break;
            case 1 : ASSERT_EQ(256u, hash); ASSERT_EQ(NULL, value); break;
            case 2 : ASSERT_EQ(456u, hash); ASSERT_EQ(NULL, value); break;
            case 3 : ASSERT_EQ(468u, hash); ASSERT_EQ(NULL, value); break;
            case 4 : ASSERT_EQ(969u, hash); ASSERT_EQ(NULL, value); break;
            default: ASSERT_EQ(0, 1); break;
        }
        ++counter;
    BSTV_END_EACH
    ASSERT_EQ(5, counter);

    bstv_destroy(bstv);
}

TEST(NAME, erase_in_for_loop)
{
    struct bstv_t* bstv = bstv_create();

    int a=79579, b=235, c=347, d=124, e=457;
    bstv_insert(bstv, 243, &a);
    bstv_insert(bstv, 256, &b);
    bstv_insert(bstv, 456, &c);
    bstv_insert(bstv, 468, &d);
    bstv_insert(bstv, 969, &e);

    BSTV_FOR_EACH(bstv, int, hash, value)
        if(hash == 256u)
            BSTV_ERASE_CURRENT_ITEM_IN_FOR_LOOP(bstv, value);
    BSTV_END_EACH

    EXPECT_THAT((int*)bstv_find(bstv, 243), Pointee(a));
    EXPECT_THAT((int*)bstv_find(bstv, 256), IsNull());
    EXPECT_THAT((int*)bstv_find(bstv, 456), Pointee(c));
    EXPECT_THAT((int*)bstv_find(bstv, 468), Pointee(d));
    EXPECT_THAT((int*)bstv_find(bstv, 969), Pointee(e));

    bstv_destroy(bstv);
}
