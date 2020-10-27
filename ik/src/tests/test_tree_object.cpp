#include "gmock/gmock.h"
#include "ik/tree_object.h"
#include "ik/cpputils.hpp"

#define NAME tree_object

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

#if !defined(NDEBUG)
TEST_F(NAME, debug_warn_if_tree_object_with_same_guid_is_added)
#else
TEST_F(NAME, ndebug_dont_warn_if_tree_object_with_same_guid_is_added)
#endif
{
    //internal::CaptureStdout();
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n3 = ik_tree_object_create_child(n2, sizeof(ik_tree_object));
    /*
#if !defined(NDEBUG)
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrNe(""));
#else
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrEq(""));
#endif*/
}

TEST_F(NAME, find_includes_root_tree_object)
{
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));            n1->user_data = (void*)1;
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));  n2->user_data = (void*)2;
    ik::Ref<ik_tree_object> n3 = ik_tree_object_create_child(n2, sizeof(ik_tree_object));  n3->user_data = (void*)3;
    EXPECT_THAT(ik_tree_object_find(n1, (void*)1), Eq(n1));
}

TEST_F(NAME, find_child_from_root)
{
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));            n1->user_data = (void*)1;
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));  n2->user_data = (void*)2;
    ik::Ref<ik_tree_object> n3 = ik_tree_object_create_child(n2, sizeof(ik_tree_object));  n3->user_data = (void*)3;
    ik::Ref<ik_tree_object> n4 = ik_tree_object_create_child(n2, sizeof(ik_tree_object));  n4->user_data = (void*)4;
    EXPECT_THAT(ik_tree_object_find(n1, (void*)4), Eq(n4));
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
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));           n1->user_data = (void*)1;
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object)); n2->user_data = (void*)2;
    ik::Ref<ik_tree_object> n3 = ik_tree_object_create_child(n2, sizeof(ik_tree_object)); n3->user_data = (void*)3;
    ik::Ref<ik_tree_object> n4 = ik_tree_object_create_child(n1, sizeof(ik_tree_object)); n4->user_data = (void*)4;
    EXPECT_THAT(ik_tree_object_find(n1, (void*)4), Eq(n4));
    EXPECT_THAT(ik_tree_object_find(n2, (void*)4), IsNull());
}

TEST_F(NAME, reparent_tree_objects_works)
{
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n3 = ik_tree_object_create_child(n2, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n4 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));
    EXPECT_THAT(ik_tree_object_child_count(n1), Eq(2u));
    EXPECT_THAT(ik_tree_object_child_count(n2), Eq(1u));
    ik_tree_object_link(n1, n3);
    EXPECT_THAT(ik_tree_object_child_count(n1), Eq(3u));
    EXPECT_THAT(ik_tree_object_child_count(n2), Eq(0u));
}

TEST_F(NAME, unlink_tree_object_works)
{
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));
    EXPECT_THAT(ik_tree_object_child_count(n1), Eq(1u));
    ik_tree_object_unlink(n2);
    EXPECT_THAT(ik_tree_object_child_count(n1), Eq(0u));
}

TEST_F(NAME, check_reparent_to_self)
{
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));
    EXPECT_THAT(ik_tree_object_can_link(n1, n1), IsFalse());
}

TEST_F(NAME, check_circular_dependency)
{
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n3 = ik_tree_object_create_child(n2, sizeof(ik_tree_object));

    EXPECT_THAT(ik_tree_object_can_link(n1, n3), IsTrue());
    EXPECT_THAT(ik_tree_object_can_link(n3, n1), IsFalse());
}

TEST_F(NAME, unlink_all_children_works)
{
    /*
     *         n1
     *        /  \
     *       n2  n4
     *      /
     *     n3
     */
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n3 = ik_tree_object_create_child(n2, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n4 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));

    EXPECT_THAT(ik_tree_object_child_count(n1), Eq(2));
    EXPECT_THAT(ik_tree_object_child_count(n2), Eq(1));

    ik_tree_object_unlink_all_children(n2);

    EXPECT_THAT(ik_tree_object_child_count(n1), Eq(2));
    EXPECT_THAT(ik_tree_object_child_count(n2), Eq(0));

    ik_tree_object_unlink_all_children(n1);

    EXPECT_THAT(ik_tree_object_child_count(n1), Eq(0));
    EXPECT_THAT(ik_tree_object_child_count(n2), Eq(0));
}

TEST_F(NAME, count_nodes_in_tree)
{
    /*
     *         n1
     *        /  \
     *       n2  n4
     *      /
     *     n3
     */
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n3 = ik_tree_object_create_child(n2, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n4 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));

    EXPECT_THAT(ik_tree_object_count(n1), Eq(4));
    EXPECT_THAT(ik_tree_object_count(n2), Eq(2));

    ik_tree_object_link(n1, n3);

    EXPECT_THAT(ik_tree_object_count(n1), Eq(4));
    EXPECT_THAT(ik_tree_object_count(n2), Eq(1));
}

TEST_F(NAME, count_leaves_in_tree)
{
    /*
     *         n1
     *        /  \
     *       n2  n4
     *      /
     *     n3
     */
    ik::Ref<ik_tree_object> n1 = ik_tree_object_create(sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n2 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n3 = ik_tree_object_create_child(n2, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n4 = ik_tree_object_create_child(n1, sizeof(ik_tree_object));

    EXPECT_THAT(ik_tree_object_leaf_count(n1), Eq(2));
    EXPECT_THAT(ik_tree_object_leaf_count(n2), Eq(1));
    EXPECT_THAT(ik_tree_object_leaf_count(n3), Eq(1));

    ik_tree_object_link(n1, n3);

    EXPECT_THAT(ik_tree_object_leaf_count(n1), Eq(3));
    EXPECT_THAT(ik_tree_object_leaf_count(n2), Eq(1));
    EXPECT_THAT(ik_tree_object_leaf_count(n3), Eq(1));
}

TEST_F(NAME, duplicate_full_no_attachments)
{
    /*
     *         n1
     *        /  \
     *       n2  n4
     *      /
     *     n3
     */
    ik::Ref<ik_tree_object> n11 = ik_tree_object_create(sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n12 = ik_tree_object_create_child(n11, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n13 = ik_tree_object_create_child(n12, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n14 = ik_tree_object_create_child(n11, sizeof(ik_tree_object));

    // Set some fields so we can check if it gets copied
    n11->mass = 2.0;            n12->mass = 3.0;            n13->mass = 4.0;            n14->mass = 5.0;
    n11->rotation_weight = 6.0; n12->rotation_weight = 7.0; n13->rotation_weight = 8.0; n14->rotation_weight = 9.0;
    n11->user_data = (void*)1;  n12->user_data = (void*)2;  n13->user_data = (void*)3;  n14->user_data = (void*)4;

    // Get copied nodes
    ik::Ref<ik_tree_object> n21 = ik_tree_object_duplicate_full(n11, sizeof(ik_tree_object), 0);
    ASSERT_THAT(n21.get(), NotNull());
    ik::Ref<ik_tree_object> n22 = ik_tree_object_find(n21, (void*)2);
    ik::Ref<ik_tree_object> n23 = ik_tree_object_find(n21, (void*)3);
    ik::Ref<ik_tree_object> n24 = ik_tree_object_find(n21, (void*)4);
    ASSERT_THAT(n22.get(), NotNull());
    ASSERT_THAT(n23.get(), NotNull());
    ASSERT_THAT(n24.get(), NotNull());

    // Check relations
    ASSERT_THAT(ik_tree_object_child_count(n21), Eq(2));
    ASSERT_THAT(ik_tree_object_child_count(n22), Eq(1));
    ASSERT_THAT(ik_tree_object_child_count(n23), Eq(0));
    ASSERT_THAT(ik_tree_object_child_count(n24), Eq(0));
    ASSERT_THAT(n24->parent, Eq(n21));
    ASSERT_THAT(n23->parent, Eq(n22));
    ASSERT_THAT(n22->parent, Eq(n21));
    ASSERT_THAT(n21->parent, IsNull());

    // Check data
    EXPECT_THAT(n21->mass, DoubleEq(2.0));
    EXPECT_THAT(n22->mass, DoubleEq(3.0));
    EXPECT_THAT(n23->mass, DoubleEq(4.0));
    EXPECT_THAT(n24->mass, DoubleEq(5.0));
    EXPECT_THAT(n21->rotation_weight, DoubleEq(6.0));
    EXPECT_THAT(n22->rotation_weight, DoubleEq(7.0));
    EXPECT_THAT(n23->rotation_weight, DoubleEq(8.0));
    EXPECT_THAT(n24->rotation_weight, DoubleEq(9.0));
    EXPECT_THAT(n21->user_data, Eq((void*)1));
    EXPECT_THAT(n22->user_data, Eq((void*)2));
    EXPECT_THAT(n23->user_data, Eq((void*)3));
    EXPECT_THAT(n24->user_data, Eq((void*)4));
}

TEST_F(NAME, duplicate_full_copies_attachments)
{
    /*
     *         n1
     *        /  \
     *       n2  n4
     *      /
     *     n3
     */
    ik::Ref<ik_tree_object> n11 = ik_tree_object_create(sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n12 = ik_tree_object_create_child(n11, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n13 = ik_tree_object_create_child(n12, sizeof(ik_tree_object));
    ik::Ref<ik_tree_object> n14 = ik_tree_object_create_child(n11, sizeof(ik_tree_object));
    n11->user_data = (void*)1;  n12->user_data = (void*)2;  n13->user_data = (void*)3;  n14->user_data = (void*)4;

    ik::Ref<ik_algorithm>  a = ik_tree_object_create_algorithm(n11, IK_FABRIK);
    ik::Ref<ik_constraint> c = ik_tree_object_create_constraint(n12);
    ik::Ref<ik_effector>   e = ik_tree_object_create_effector(n13);
    ik::Ref<ik_pole>       p = ik_tree_object_create_pole(n14);

    // Get copied nodes
    ik::Ref<ik_tree_object> n21 = ik_tree_object_duplicate_full(n11, sizeof(ik_tree_object), 0);
    ASSERT_THAT(n21.get(), NotNull());
    ik::Ref<ik_tree_object> n22 = ik_tree_object_find(n21, (void*)2);
    ik::Ref<ik_tree_object> n23 = ik_tree_object_find(n21, (void*)3);
    ik::Ref<ik_tree_object> n24 = ik_tree_object_find(n21, (void*)4);
    ASSERT_THAT(n22.get(), NotNull());
    ASSERT_THAT(n23.get(), NotNull());
    ASSERT_THAT(n24.get(), NotNull());

    EXPECT_THAT(n21->algorithm, NotNull());
    EXPECT_THAT(n22->algorithm, IsNull());
    EXPECT_THAT(n23->algorithm, IsNull());
    EXPECT_THAT(n24->algorithm, IsNull());

    EXPECT_THAT(n21->constraint, IsNull());
    EXPECT_THAT(n22->constraint, NotNull());
    EXPECT_THAT(n23->constraint, IsNull());
    EXPECT_THAT(n24->constraint, IsNull());

    EXPECT_THAT(n21->effector, IsNull());
    EXPECT_THAT(n22->effector, IsNull());
    EXPECT_THAT(n23->effector, NotNull());
    EXPECT_THAT(n24->effector, IsNull());

    EXPECT_THAT(n21->pole, IsNull());
    EXPECT_THAT(n22->pole, IsNull());
    EXPECT_THAT(n23->pole, IsNull());
    EXPECT_THAT(n24->pole, NotNull());
}

TEST_F(NAME, destroying_parent_unlinks_children)
{
    ik_tree_object* parent = ik_tree_object_create(sizeof(ik_tree_object));
    ik_tree_object* child1 = ik_tree_object_create_child(parent, sizeof(ik_tree_object));
    ik_tree_object* child2 = ik_tree_object_create_child(parent, sizeof(ik_tree_object));

    IK_INCREF(parent);
    IK_INCREF(child1);
    IK_INCREF(child2);

    EXPECT_THAT(child1->parent, Eq(parent));
    EXPECT_THAT(child2->parent, Eq(parent));

    IK_DECREF(parent);  // destroys parent

    EXPECT_THAT(child1->parent, IsNull());
    EXPECT_THAT(child2->parent, IsNull());

    IK_DECREF(child1);
    IK_DECREF(child2);
}
