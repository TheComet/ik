#include "gmock/gmock.h"
#include "ik/node.h"
#include "ik/cpputils.hpp"

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
    ik::Ref<ik_node> n = ik_node_create(ik_guid(3));
    EXPECT_THAT(n->user.guid, Eq(3));

    n = ik_node_create(ik_guid(56));
    EXPECT_THAT(n->user.guid, Eq(56));
}

#ifdef DEBUG
TEST_F(NAME, debug_warn_if_node_with_same_guid_is_added)
#else
TEST_F(NAME, ndebug_dont_warn_if_node_with_same_guid_is_added)
#endif
{
    //internal::CaptureStdout();
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(2));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(3));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(2));
    /*
#ifdef DEBUG
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrNe(""));
#else
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrEq(""));
#endif*/
}

TEST_F(NAME, error_if_child_nodes_have_same_guid)
{
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(2));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(3));
    ik::Ref<ik_node> n3 = ik_node_create(ik_guid(3));

    // Test both link and create_child
    EXPECT_THAT(ik_node_link(n1, n3), Ne(0));
    ik::Ref<ik_node> n4 = ik_node_create_child(n1, ik_guid(3));
    EXPECT_THAT(n4.isNull(), IsTrue());
}

TEST_F(NAME, child_keeps_parent_if_link_fails)
{
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(2));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(3));

    ik::Ref<ik_node> n3 = ik_node_create(ik_guid(2));
    ik::Ref<ik_node> n4 = ik_node_create_child(n3, ik_guid(3));

    EXPECT_THAT(n4->parent, Eq(n3));
    EXPECT_THAT(ik_node_link(n1, n4), Eq(-1));
    EXPECT_THAT(n4->parent, Eq(n3));
}

TEST_F(NAME, find_includes_root_node)
{
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(2));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(3));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(4));
    EXPECT_THAT(ik_node_find(n1, ik_guid(2)), Eq(n1));
}

TEST_F(NAME, find_child_from_root)
{
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(2));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(3));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(4));
    ik::Ref<ik_node> n4 = ik_node_create_child(n2, ik_guid(5));
    EXPECT_THAT(ik_node_find(n1, ik_guid(5)), Eq(n4));
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
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(2));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(3));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(4));
    ik::Ref<ik_node> n4 = ik_node_create_child(n1, ik_guid(5));
    EXPECT_THAT(ik_node_find(n1, ik_guid(5)), Eq(n4));
    EXPECT_THAT(ik_node_find(n2, ik_guid(5)), IsNull());
}

TEST_F(NAME, reparent_nodes_works)
{
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(2));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(3));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(4));
    ik::Ref<ik_node> n4 = ik_node_create_child(n1, ik_guid(5));
    EXPECT_THAT(ik_node_child_count(n1), Eq(2u));
    EXPECT_THAT(ik_node_child_count(n2), Eq(1u));
    ik_node_link(n1, n3);
    EXPECT_THAT(ik_node_child_count(n1), Eq(3u));
    EXPECT_THAT(ik_node_child_count(n2), Eq(0u));
}

TEST_F(NAME, unlink_node_works)
{
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(2));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(3));
    EXPECT_THAT(ik_node_child_count(n1), Eq(1u));
    ik_node_unlink(n2);
    EXPECT_THAT(ik_node_child_count(n1), Eq(0u));
}

TEST_F(NAME, check_reparent_to_self)
{
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(2));
    EXPECT_THAT(ik_node_can_link(n1, n1), IsFalse());
}

TEST_F(NAME, link_NULL_death)
{
    ik::Ref<ik_node> n1 = ik_node_create(ik_guid(1));
    ik::Ref<ik_node> n2 = ik_node_create(ik_guid(2));
    EXPECT_DEATH(ik_node_link(n1, NULL), ".*");
    EXPECT_DEATH(ik_node_link(NULL, n2), ".*");
    EXPECT_DEATH(ik_node_link(NULL, NULL), ".*");
}

TEST_F(NAME, create_child_NULL_death)
{
    EXPECT_DEATH(ik_node_create_child(NULL, ik_guid(1)), ".*");
}

TEST_F(NAME, unlink_NULL_death)
{
    EXPECT_DEATH(ik_node_unlink(NULL), ".*");
}
