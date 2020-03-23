#include "gmock/gmock.h"
#include "ik/refcount.h"
#include "cstructures/memory.h"

#define NAME refcount

using namespace ::testing;

struct MyObj {
    IK_REFCOUNT_HEAD
    int x, y, z;
};

TEST(NAME, can_decref_with_null_deinit_function)
{
    MyObj* o = (MyObj*)ik_refcounted_alloc(sizeof(MyObj), NULL);
    ASSERT_THAT(o, NotNull());
    EXPECT_THAT(IK_REFCOUNT(o), Eq(1));
    IK_DECREF(o);
}

static bool deinit1Called = false;
static void deinit1(void* o)
{
    deinit1Called = true;
}
TEST(NAME, deinit_function_is_called_on_decref)
{
    MyObj* o = (MyObj*)ik_refcounted_alloc(sizeof(MyObj), deinit1);
    ASSERT_THAT(o, NotNull());
    deinit1Called = false;
    IK_DECREF(o);
    EXPECT_THAT(deinit1Called, IsTrue());
}

static bool deinit2Called = false;
static void deinit2(void* o)
{
    deinit2Called = true;
}
TEST(NAME, deinit_function_is_only_called_on_last_decref)
{
    MyObj *o;
    EXPECT_THAT(ik_refcounted_alloc((ik_refcounted_t**)&o, sizeof(MyObj), deinit2), Eq(IK_OK));
    IK_INCREF(o);
    deinit2Called = false;
    IK_DECREF(o);
    EXPECT_THAT(deinit2Called, IsFalse());
    IK_DECREF(o);
    EXPECT_THAT(deinit2Called, IsTrue());
}

static void* deinit3Ptrs[5];
static int deinit3Calls = 0;
static void deinit3(void* o)
{
    deinit3Ptrs[deinit3Calls++] = o;
}
TEST(NAME, deinit_function_is_called_on_every_element_in_array)
{
    MyObj* o;
    EXPECT_THAT(ik_refcounted_alloc_array((ik_refcounted_t**)&o, sizeof(MyObj), deinit3, 5), Eq(IK_OK));
    deinit3Calls = 0;
    memset(deinit3Ptrs, 0, sizeof(void*) * 5);
    IK_DECREF(o);
    EXPECT_THAT(deinit3Calls, Eq(5));
    EXPECT_THAT((MyObj*)deinit3Ptrs[0], Eq(o));
    EXPECT_THAT((MyObj*)deinit3Ptrs[1], Eq(o + 1));
    EXPECT_THAT((MyObj*)deinit3Ptrs[2], Eq(o + 2));
    EXPECT_THAT((MyObj*)deinit3Ptrs[3], Eq(o + 3));
    EXPECT_THAT((MyObj*)deinit3Ptrs[4], Eq(o + 4));
}
