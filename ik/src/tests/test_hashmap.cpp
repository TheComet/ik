#include <gmock/gmock.h>
#include "ik/hashmap.h"

#define NAME hashmap

using namespace ::testing;

static const char KEY1[16] = "KEY1";
static const char KEY2[16] = "KEY2";
static const char KEY3[16] = "KEY3";
static const char KEY4[16] = "KEY4";
static const char KEY5[16] = "KEY5";

static hash32_t shitty_hash(const void* data, uintptr_t len)
{
    return 42;
}
static hash32_t collide_with_shitty_hash(const void* data, uintptr_t len)
{
    return HM_DEFAULT_TABLE_COUNT + 42;
}
static hash32_t collide_with_shitty_hash_second_probe(const void* data, uintptr_t len)
{
    return HM_DEFAULT_TABLE_COUNT + 45; // sequence would be 42, 43, 45, 48, ...
}

class NAME : public Test
{
protected:
    hashmap_t* hm;

public:
    virtual void SetUp()
    {
        ASSERT_THAT(hashmap_create(&hm, 16, sizeof(float)), Eq(IK_OK));
    }

    virtual void TearDown()
    {
        hashmap_destroy(hm);
    }
};

TEST_F(NAME, construct_sane_values)
{
    EXPECT_THAT(hm->table_count, Eq(HM_DEFAULT_TABLE_COUNT));
    EXPECT_THAT(hm->key_size, Eq(16));
    EXPECT_THAT(hm->value_size, Eq(sizeof(float)));
    EXPECT_THAT(hashmap_count(hm), Eq(0));
    EXPECT_THAT(hm->storage, NotNull());
}

TEST_F(NAME, insert_increases_slots_used)
{
    float f = 5.6f;
    EXPECT_THAT(hashmap_count(hm), Eq(0));
    EXPECT_THAT(hashmap_insert(hm, KEY1, &f), Eq(IK_OK));
    EXPECT_THAT(hashmap_count(hm), Eq(1));
}

TEST_F(NAME, erase_decreases_slots_used)
{
    float f = 5.6f;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &f), Eq(IK_OK));
    EXPECT_THAT(hashmap_count(hm), Eq(1));
    EXPECT_THAT(hashmap_erase(hm, KEY1), NotNull());
    EXPECT_THAT(hashmap_count(hm), Eq(0));
}

TEST_F(NAME, erase_returns_value)
{
    float f = 5.6f;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &f), Eq(IK_OK));
    EXPECT_THAT(*(float*)hashmap_erase(hm, KEY1), FloatEq(f));
}

TEST_F(NAME, insert_same_key_twice_only_works_once)
{
    float f = 5.6f;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &f), Eq(IK_OK));
    EXPECT_THAT(hashmap_insert(hm, KEY1, &f), Eq(WS_KEY_EXISTS));
    EXPECT_THAT(hashmap_count(hm), Eq(1));
}

TEST_F(NAME, erasing_same_key_twice_only_works_once)
{
    float f = 5.6f;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &f), Eq(IK_OK));
    EXPECT_THAT(hashmap_erase(hm, KEY1), NotNull());
    EXPECT_THAT(hashmap_erase(hm, KEY1), IsNull());
    EXPECT_THAT(hashmap_count(hm), Eq(0));
}

TEST_F(NAME, hash_collision_insert_ab_erase_ba)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &a), Eq(IK_OK));
    EXPECT_THAT(hashmap_insert(hm, KEY2, &b), Eq(IK_OK));
    EXPECT_THAT(hashmap_count(hm), Eq(2));
    EXPECT_THAT(*(float*)hashmap_erase(hm, KEY2), FloatEq(b));
    EXPECT_THAT(*(float*)hashmap_erase(hm, KEY1), FloatEq(a));
    EXPECT_THAT(hashmap_count(hm), Eq(0));
}

TEST_F(NAME, hash_collision_insert_ab_erase_ab)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &a), Eq(IK_OK));
    EXPECT_THAT(hashmap_insert(hm, KEY2, &b), Eq(IK_OK));
    EXPECT_THAT(hashmap_count(hm), Eq(2));
    EXPECT_THAT(*(float*)hashmap_erase(hm, KEY1), FloatEq(a));
    EXPECT_THAT(*(float*)hashmap_erase(hm, KEY2), FloatEq(b));
    EXPECT_THAT(hashmap_count(hm), Eq(0));
}

TEST_F(NAME, hash_collision_insert_ab_find_ab)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &a), Eq(IK_OK));
    EXPECT_THAT(hashmap_insert(hm, KEY2, &b), Eq(IK_OK));
    EXPECT_THAT(*(float*)hashmap_find(hm, KEY1), FloatEq(a));
    EXPECT_THAT(*(float*)hashmap_find(hm, KEY2), FloatEq(b));
}

TEST_F(NAME, hash_collision_insert_ab_erase_a_find_b)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &a), Eq(IK_OK));
    EXPECT_THAT(hashmap_insert(hm, KEY2, &b), Eq(IK_OK));
    EXPECT_THAT(hashmap_erase(hm, KEY1), NotNull());
    EXPECT_THAT(*(float*)hashmap_find(hm, KEY2), FloatEq(b));
}

TEST_F(NAME, hash_collision_insert_ab_erase_b_find_a)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &a), Eq(IK_OK));
    EXPECT_THAT(hashmap_insert(hm, KEY2, &b), Eq(IK_OK));
    EXPECT_THAT(hashmap_erase(hm, KEY2), NotNull());
    EXPECT_THAT(*(float*)hashmap_find(hm, KEY1), FloatEq(a));
}

TEST_F(NAME, hash_collision_insert_at_tombstone)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &a), Eq(IK_OK));
    EXPECT_THAT(hashmap_insert(hm, KEY2, &b), Eq(IK_OK));
    EXPECT_THAT(hashmap_count(hm), Eq(2));
    EXPECT_THAT(*(float*)hashmap_erase(hm, KEY1), FloatEq(a)); // creates tombstone
    EXPECT_THAT(hashmap_count(hm), Eq(1));
    EXPECT_THAT(hashmap_insert(hm, KEY1, &a), Eq(IK_OK)); // should insert at tombstone location
    EXPECT_THAT(*(float*)hashmap_erase(hm, KEY1), FloatEq(a));
    EXPECT_THAT(*(float*)hashmap_erase(hm, KEY2), FloatEq(b));
    EXPECT_THAT(hashmap_count(hm), Eq(0));
}

TEST_F(NAME, hash_collision_insert_at_tombstone_with_existing_key)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hashmap_insert(hm, KEY1, &a), Eq(IK_OK));
    EXPECT_THAT(hashmap_insert(hm, KEY2, &b), Eq(IK_OK));
    EXPECT_THAT(hashmap_count(hm), Eq(2));
    EXPECT_THAT(*(float*)hashmap_erase(hm, KEY1), FloatEq(a)); // creates tombstone
    EXPECT_THAT(hashmap_count(hm), Eq(1));
    EXPECT_THAT(hashmap_insert(hm, KEY2, &a), Eq(WS_KEY_EXISTS));
    EXPECT_THAT(hashmap_count(hm), Eq(1));
}

TEST_F(NAME, remove_probing_sequence_scenario_1)
{
    float a = 5.6f;
    float b = 3.4f;
    // Creates a tombstone in the probing sequence to KEY2
    hm->hash = shitty_hash;
    hashmap_insert(hm, KEY1, &a);
    hashmap_insert(hm, KEY2, &b);
    hashmap_erase(hm, KEY1);

    // Inserts a different hash into where the tombstone is
    hm->hash = collide_with_shitty_hash;
    hashmap_insert(hm, KEY1, &a);
    hm->hash = shitty_hash;

    // Does this cut off access to KEY2?
    /*ASSERT_THAT(hashmap_find(hm, KEY2), NotNull());
    EXPECT_THAT(*(float*)hashmap_find(hm, KEY2), FloatEq(b));*/
    float* ret = (float*)hashmap_erase(hm, KEY2);
    ASSERT_THAT(ret, NotNull());
    EXPECT_THAT(*ret, FloatEq(b));
}

TEST_F(NAME, remove_probing_sequence_scenario_2)
{
    float a = 5.6f;
    float b = 3.4f;
    float c = 1.8f;
    float d = 8.7f;

    // First key is inserted directly, next 2 collide and are inserted along the probing sequence
    hm->hash = shitty_hash;
    hashmap_insert(hm, KEY1, &a);
    hashmap_insert(hm, KEY2, &b);
    hashmap_insert(hm, KEY3, &c);

    // Insert a key with a different hash that collides with the slot of KEY3
    hm->hash = collide_with_shitty_hash_second_probe;
    hashmap_insert(hm, KEY4, &d);

    // Erase KEY3
    hm->hash = shitty_hash;  // restore shitty hash
    hashmap_erase(hm, KEY3);

    // Does this cut off access to KEY4?
    hm->hash = collide_with_shitty_hash_second_probe;
    float* ret = (float*)hashmap_erase(hm, KEY4);
    ASSERT_THAT(ret, NotNull());
    EXPECT_THAT(*ret, FloatEq(d));
}

TEST_F(NAME, rehash_test)
{
    char key[16];
    float value = 0;
    for (int i = 0; i != HM_DEFAULT_TABLE_COUNT*128; ++i, value += 1.5f)
    {
        memset(key, 0, sizeof key);
        sprintf(key, "%d", i);
        ASSERT_THAT(hashmap_insert(hm, key, &value), Eq(IK_OK));
    }

    value = 0;
    for (int i = 0; i != HM_DEFAULT_TABLE_COUNT*128; ++i, value += 1.5f)
    {
        memset(key, 0, sizeof key);
        sprintf(key, "%d", i);
        float* retvalue = (float*)hashmap_erase(hm, key);
        EXPECT_THAT(retvalue, NotNull());
        if (retvalue != NULL)
            EXPECT_THAT(*retvalue, FloatEq(value));
        else
            printf("hashmap_erase() returned NULL for key %s\n", key);
    }

}
