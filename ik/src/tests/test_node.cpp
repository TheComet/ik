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
};

TEST_F(NAME, create_sets_proper_guid)
{
    ik_node_t* n;
    IKAPI.node.create(&n, IKAPI.to_ptr(3));
    EXPECT_THAT(IKAPI.node.get_uid(n), Eq(3));
    IKAPI.node.destroy(n);
    IKAPI.node.create(&n, IKAPI.to_ptr(56));
    EXPECT_THAT(IKAPI.node.get_uid(n), Eq(56));
    IKAPI.node.destroy(n);
}

TEST_F(NAME, construct_sets_proper_guid)
{
    ik_node_t n;
    IKAPI.node.construct(&n, IKAPI.to_ptr(5));
    EXPECT_THAT(IKAPI.node.get_uid(&n), Eq(5));
    IKAPI.node.destruct(&n);
    IKAPI.node.construct(&n, IKAPI.to_ptr(54));
    EXPECT_THAT(IKAPI.node.get_uid(&n), Eq(54));
    IKAPI.node.destruct(&n);
}

#ifdef DEBUG
TEST_F(NAME, debug_warn_if_node_with_same_guid_is_added)
#else
TEST_F(NAME, ndebug_dont_warn_if_node_with_same_guid_is_added)
#endif
{
    internal::CaptureStdout();
    ik_node_t* n1; IKAPI.node.create(&n1, IKAPI.to_ptr(2));
    ik_node_t* n2; IKAPI.node.create_child(&n2, n1, IKAPI.to_ptr(3));
    ik_node_t* n3; IKAPI.node.create_child(&n3, n2, IKAPI.to_ptr(2));
#ifdef DEBUG
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrNe(""));
#else
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrEq(""));
#endif
    IKAPI.node.destroy_recursive(n1);
}

TEST_F(NAME, error_if_child_nodes_have_same_guid)
{
    ik_node_t* n1; IKAPI.node.create(&n1, IKAPI.to_ptr(2));
    ik_node_t* n2; IKAPI.node.create_child(&n2, n1, IKAPI.to_ptr(3));
    ik_node_t* n3; IKAPI.node.create(&n3, IKAPI.to_ptr(3));
    ik_node_t* n4;
    // Test both link and create_child
    EXPECT_THAT(IKAPI.node.link(n1, n3), Eq(IK_HASH_EXISTS));
    EXPECT_THAT(IKAPI.node.create_child(&n4, n1, IKAPI.to_ptr(3)), Eq(IK_HASH_EXISTS));
    IKAPI.node.destroy_recursive(n1);
    IKAPI.node.destroy_recursive(n3);
}

TEST_F(NAME, find_child_includes_root_node)
{
    ik_node_t* n1; IKAPI.node.create(&n1, IKAPI.to_ptr(2));
    ik_node_t* n2; IKAPI.node.create_child(&n2, n1, IKAPI.to_ptr(3));
    ik_node_t* n3; IKAPI.node.create_child(&n3, n2, IKAPI.to_ptr(4));
    ik_node_t* find;
    EXPECT_THAT(IKAPI.node.find_child(&find, n1, IKAPI.to_ptr(2)), Eq(IK_OK));
    EXPECT_THAT(find, NotNull());
    IKAPI.node.destroy_recursive(n1);
}

TEST_F(NAME, find_child_from_root)
{
    ik_node_t* n1; IKAPI.node.create(&n1, IKAPI.to_ptr(2));
    ik_node_t* n2; IKAPI.node.create_child(&n2, n1, IKAPI.to_ptr(3));
    ik_node_t* n3; IKAPI.node.create_child(&n3, n2, IKAPI.to_ptr(4));
    ik_node_t* n4; IKAPI.node.create_child(&n4, n2, IKAPI.to_ptr(5));
    ik_node_t* find;
    EXPECT_THAT(IKAPI.node.find_child(&find, n1, IKAPI.to_ptr(5)), Eq(IK_OK));
    IKAPI.node.destroy_recursive(n1);
}

TEST_F(NAME, find_child_doesnt_find_parents)
{
    /*
     *         n1
     *        /  \
     *       n2  n4
     *      /
     *     n3
     */
    ik_node_t* n1; IKAPI.node.create(&n1, IKAPI.to_ptr(2));
    ik_node_t* n2; IKAPI.node.create_child(&n2, n1, IKAPI.to_ptr(3));
    ik_node_t* n3; IKAPI.node.create_child(&n3, n2, IKAPI.to_ptr(4));
    ik_node_t* n4; IKAPI.node.create_child(&n4, n1, IKAPI.to_ptr(5));
    ik_node_t* find;
    EXPECT_THAT(IKAPI.node.find_child(&find, n1, IKAPI.to_ptr(5)), Eq(IK_OK));
    EXPECT_THAT(IKAPI.node.find_child(&find, n2, IKAPI.to_ptr(5)), Eq(IK_NODE_NOT_FOUND));
    IKAPI.node.destroy_recursive(n1);
}

TEST_F(NAME, reparent_nodes_works)
{
    ik_node_t* n1; IKAPI.node.create(&n1, IKAPI.to_ptr(2));
    ik_node_t* n2; IKAPI.node.create_child(&n2, n1, IKAPI.to_ptr(3));
    ik_node_t* n3; IKAPI.node.create_child(&n3, n2, IKAPI.to_ptr(4));
    ik_node_t* n4; IKAPI.node.create_child(&n4, n1, IKAPI.to_ptr(5));
    EXPECT_THAT(IKAPI.node.child_count(n1), Eq(2u));
    EXPECT_THAT(IKAPI.node.child_count(n2), Eq(1u));
    IKAPI.node.link(n1, n3);
    EXPECT_THAT(IKAPI.node.child_count(n1), Eq(3u));
    EXPECT_THAT(IKAPI.node.child_count(n2), Eq(0u));
    IKAPI.node.destroy_recursive(n1);
}

TEST_F(NAME, unlink_node_works)
{
    ik_node_t* n1; IKAPI.node.create(&n1, IKAPI.to_ptr(2));
    ik_node_t* n2; IKAPI.node.create_child(&n2, n1, IKAPI.to_ptr(3));
    EXPECT_THAT(IKAPI.node.child_count(n1), Eq(1u));
    IKAPI.node.unlink(n2);
    EXPECT_THAT(IKAPI.node.child_count(n1), Eq(0u));
    IKAPI.node.destroy(n1);
    IKAPI.node.destroy(n2);
}

TEST_F(NAME, reparent_to_self_causes_death)
{
    ik_node_t* n1; IKAPI.node.create(&n1, IKAPI.to_ptr(2));
    EXPECT_DEATH(IKAPI.node.link(n1, n1), ".*");
    IKAPI.node.destroy(n1);
}

TEST_F(NAME, construct_NULL_death)
{
    EXPECT_DEATH(IKAPI.node.construct(NULL, 0), ".*");
}

TEST_F(NAME, destruct_and_destroy_NULL_death)
{
    EXPECT_DEATH(IKAPI.node.destruct(NULL), ".*");
    EXPECT_DEATH(IKAPI.node.destroy(NULL), ".*");
}

TEST_F(NAME, link_NULL_death)
{
    ik_node_t* n1; IKAPI.node.create(&n1, IKAPI.to_ptr(1));
    ik_node_t* n2; IKAPI.node.create(&n2, IKAPI.to_ptr(2));
    EXPECT_DEATH(IKAPI.node.link(n1, NULL), ".*");
    EXPECT_DEATH(IKAPI.node.link(NULL, n2), ".*");
    EXPECT_DEATH(IKAPI.node.link(NULL, NULL), ".*");
    IKAPI.node.destroy(n1);
    IKAPI.node.destroy(n2);
}

TEST_F(NAME, create_child_NULL_death)
{
    ik_node_t* n;
    EXPECT_DEATH(IKAPI.node.create_child(&n, NULL, IKAPI.to_ptr(1)), ".*");
}

TEST_F(NAME, unlink_NULL_death)
{
    EXPECT_DEATH(IKAPI.node.unlink(NULL), ".*");
}
