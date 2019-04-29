#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME node

using namespace ::testing;

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

    void* to_ptr(uintptr_t i) { return (void*)i; }
    uintptr_t to_id(const void* p) { return (uintptr_t)p; }
};

TEST_F(NAME, create_sets_proper_guid)
{
    ik_node_t* n;
    ik_node_create(&n, to_ptr(3));
    EXPECT_THAT(to_id(IK_NODE_USER_DATA(n)), Eq(3));
    ik_node_free(n);
    ik_node_create(&n, to_ptr(56));
    EXPECT_THAT(to_id(IK_NODE_USER_DATA(n)), Eq(56));
    ik_node_free(n);
}

TEST_F(NAME, construct_sets_proper_guid)
{
    ik_node_t n;
    ik_node_init(&n, to_ptr(5));
    EXPECT_THAT(to_id(IK_NODE_USER_DATA(&n)), Eq(5));
    ik_node_deinit(&n);
    ik_node_init(&n, to_ptr(54));
    EXPECT_THAT(to_id(IK_NODE_USER_DATA(&n)), Eq(54));
    ik_node_deinit(&n);
}

#ifdef DEBUG
TEST_F(NAME, debug_warn_if_node_with_same_guid_is_added)
#else
TEST_F(NAME, ndebug_dont_warn_if_node_with_same_guid_is_added)
#endif
{
    internal::CaptureStdout();
    ik_node_t* n1; ik_node_create(&n1, to_ptr(2));
    ik_node_t* n2; ik_node_create_child(&n2, n1, to_ptr(3));
    ik_node_t* n3; ik_node_create_child(&n3, n2, to_ptr(2));
#ifdef DEBUG
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrNe(""));
#else
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrEq(""));
#endif
    ik_node_free_recursive(n1);
}

TEST_F(NAME, error_if_child_nodes_have_same_guid)
{
    ik_node_t* n1; ik_node_create(&n1, to_ptr(2));
    ik_node_t* n2; ik_node_create_child(&n2, n1, to_ptr(3));
    ik_node_t* n3; ik_node_create(&n3, to_ptr(3));
    ik_node_t* n4;
    // Test both link and create_child
    EXPECT_THAT(ik_node_link(n1, n3), Eq(IK_ERR_DUPLICATE_NODE));
    EXPECT_THAT(ik_node_create_child(&n4, n1, to_ptr(3)), Eq(IK_ERR_DUPLICATE_NODE));
    ik_node_free_recursive(n1);
    ik_node_free_recursive(n3);
}

TEST_F(NAME, find_includes_root_node)
{
    ik_node_t* n1; ik_node_create(&n1, to_ptr(2));
    ik_node_t* n2; ik_node_create_child(&n2, n1, to_ptr(3));
    ik_node_t* n3; ik_node_create_child(&n3, n2, to_ptr(4));
    EXPECT_THAT(ik_node_find(n1, to_ptr(2)), Eq(n1));
    ik_node_free_recursive(n1);
}

TEST_F(NAME, find_child_from_root)
{
    ik_node_t* n1; ik_node_create(&n1, to_ptr(2));
    ik_node_t* n2; ik_node_create_child(&n2, n1, to_ptr(3));
    ik_node_t* n3; ik_node_create_child(&n3, n2, to_ptr(4));
    ik_node_t* n4; ik_node_create_child(&n4, n2, to_ptr(5));
    EXPECT_THAT(ik_node_find(n1, to_ptr(5)), Eq(n4));
    ik_node_free_recursive(n1);
}

TEST_F(NAME, find_doesnt_find_parents)
{
    /*
     *         n1
     *        /  \
     *       n2  n4
     *      /
     *     n3
     */
    ik_node_t* n1; ik_node_create(&n1, to_ptr(2));
    ik_node_t* n2; ik_node_create_child(&n2, n1, to_ptr(3));
    ik_node_t* n3; ik_node_create_child(&n3, n2, to_ptr(4));
    ik_node_t* n4; ik_node_create_child(&n4, n1, to_ptr(5));
    EXPECT_THAT(ik_node_find(n1, to_ptr(5)), Eq(n4));
    EXPECT_THAT(ik_node_find(n2, to_ptr(5)), IsNull());
    ik_node_free_recursive(n1);
}

TEST_F(NAME, reparent_nodes_works)
{
    ik_node_t* n1; ik_node_create(&n1, to_ptr(2));
    ik_node_t* n2; ik_node_create_child(&n2, n1, to_ptr(3));
    ik_node_t* n3; ik_node_create_child(&n3, n2, to_ptr(4));
    ik_node_t* n4; ik_node_create_child(&n4, n1, to_ptr(5));
    EXPECT_THAT(IK_NODE_CHILD_COUNT(n1), Eq(2u));
    EXPECT_THAT(IK_NODE_CHILD_COUNT(n2), Eq(1u));
    ik_node_link(n1, n3);
    EXPECT_THAT(IK_NODE_CHILD_COUNT(n1), Eq(3u));
    EXPECT_THAT(IK_NODE_CHILD_COUNT(n2), Eq(0u));
    ik_node_free_recursive(n1);
}

TEST_F(NAME, unlink_node_works)
{
    ik_node_t* n1; ik_node_create(&n1, to_ptr(2));
    ik_node_t* n2; ik_node_create_child(&n2, n1, to_ptr(3));
    EXPECT_THAT(IK_NODE_CHILD_COUNT(n1), Eq(1u));
    ik_node_unlink(n2);
    EXPECT_THAT(IK_NODE_CHILD_COUNT(n1), Eq(0u));
    ik_node_free(n1);
    ik_node_free(n2);
}

TEST_F(NAME, reparent_to_self_causes_death)
{
    ik_node_t* n1; ik_node_create(&n1, to_ptr(2));
    EXPECT_DEATH(ik_node_link(n1, n1), ".*");
    ik_node_free(n1);
}

TEST_F(NAME, construct_NULL_death)
{
    EXPECT_DEATH(ik_node_init(NULL, 0), ".*");
}

TEST_F(NAME, destruct_and_destroy_NULL_death)
{
    EXPECT_DEATH(ik_node_deinit(NULL), ".*");
    EXPECT_DEATH(ik_node_free(NULL), ".*");
}

TEST_F(NAME, link_NULL_death)
{
    ik_node_t* n1; ik_node_create(&n1, to_ptr(1));
    ik_node_t* n2; ik_node_create(&n2, to_ptr(2));
    EXPECT_DEATH(ik_node_link(n1, NULL), ".*");
    EXPECT_DEATH(ik_node_link(NULL, n2), ".*");
    EXPECT_DEATH(ik_node_link(NULL, NULL), ".*");
    ik_node_free(n1);
    ik_node_free(n2);
}

TEST_F(NAME, create_child_NULL_death)
{
    ik_node_t* n;
    EXPECT_DEATH(ik_node_create_child(&n, NULL, to_ptr(1)), ".*");
}

TEST_F(NAME, unlink_NULL_death)
{
    EXPECT_DEATH(ik_node_unlink(NULL), ".*");
}
