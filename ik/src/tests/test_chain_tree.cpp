#include "gmock/gmock.h"
#include "ik/chain_tree.h"
#include "ik/bone.h"
#include "ik/subtree.h"
#include "ik/cpputils.hpp"

#define NAME chain

using namespace ::testing;

class NAME : public Test
{
public:
    virtual void SetUp()
    {
        subtree_init(&subtree);
        chain_tree_init(&chain_tree);
    }

    virtual void TearDown()
    {
        chain_tree_deinit(&chain_tree);
        subtree_deinit(&subtree);
    }

protected:
    ik_subtree subtree;
    ik_chain chain_tree;
};

TEST_F(NAME, single_bone)
{
    ik::Ref<ik_bone> bone = ik_bone_create();
    subtree_set_root(&subtree, bone);
    subtree_add_leaf(&subtree, bone);

    EXPECT_THAT(chain_tree_build(&chain_tree, &subtree), Eq(0));

    EXPECT_THAT(chain_bone_count(&chain_tree), Eq(1));
    EXPECT_THAT(chain_get_base_bone(&chain_tree), Eq(bone));
    EXPECT_THAT(chain_get_tip_bone(&chain_tree), Eq(bone));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
}

TEST_F(NAME, two_bones)
{
    ik::Ref<ik_bone> n0 = ik_bone_create();
    ik::Ref<ik_bone> n1 = ik_bone_create_child(n0);

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n1);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain_tree), Eq(2));
    EXPECT_THAT(chain_get_base_bone(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_bone(&chain_tree), Eq(n1));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
}

TEST_F(NAME, omit_first_and_last)
{
    ik::Ref<ik_bone> n0 = ik_bone_create();
    ik::Ref<ik_bone> n1 = ik_bone_create_child(n0);
    ik::Ref<ik_bone> n2 = ik_bone_create_child(n1);
    ik::Ref<ik_bone> n3 = ik_bone_create_child(n2);
    ik::Ref<ik_bone> n4 = ik_bone_create_child(n3);

    subtree_set_root(&subtree, n1);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain_tree), Eq(3));
    EXPECT_THAT(chain_get_base_bone(&chain_tree), Eq(n1));
    EXPECT_THAT(chain_get_tip_bone(&chain_tree), Eq(n3));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));

    // Check refcounts
    EXPECT_THAT(n0.refcount(), Eq(1));
    EXPECT_THAT(n1.refcount(), Eq(3));
    EXPECT_THAT(n2.refcount(), Eq(3));
    EXPECT_THAT(n3.refcount(), Eq(3));
    EXPECT_THAT(n4.refcount(), Eq(2));
}

TEST_F(NAME, ignore_branch_not_part_of_subtree)
{
    ik::Ref<ik_bone> n0 = ik_bone_create();
    ik::Ref<ik_bone> n1 = ik_bone_create_child(n0);
    ik::Ref<ik_bone> n2 = ik_bone_create_child(n1);
    ik::Ref<ik_bone> n3 = ik_bone_create_child(n2);
    ik::Ref<ik_bone> n4 = ik_bone_create_child(n2);

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain_tree), Eq(4));
    EXPECT_THAT(chain_get_base_bone(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_bone(&chain_tree), Eq(n3));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
}

TEST_F(NAME, children_of_leaf_bones_are_not_added_as_dead_bones)
{
    ik::Ref<ik_bone> n0 = ik_bone_create();
    ik::Ref<ik_bone> n1 = ik_bone_create_child(n0);
    ik::Ref<ik_bone> n2 = ik_bone_create_child(n1);
    ik::Ref<ik_bone> n3 = ik_bone_create_child(n2);
    ik::Ref<ik_bone> n4 = ik_bone_create_child(n3);
    ik::Ref<ik_bone> n5 = ik_bone_create_child(n3);

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain_tree), Eq(4));
    EXPECT_THAT(chain_get_base_bone(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_bone(&chain_tree), Eq(n3));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
}

TEST_F(NAME, children_of_base_bone_are_not_added_as_dead_bones)
{
    ik::Ref<ik_bone> n0 = ik_bone_create();
    ik::Ref<ik_bone> n1 = ik_bone_create_child(n0);
    ik::Ref<ik_bone> n2 = ik_bone_create_child(n1);
    ik::Ref<ik_bone> n3 = ik_bone_create_child(n2);
    ik::Ref<ik_bone> n4 = ik_bone_create_child(n0);

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain_tree), Eq(4));
    EXPECT_THAT(chain_get_base_bone(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_bone(&chain_tree), Eq(n3));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
}

TEST_F(NAME, two_arms)
{
    ik::Ref<ik_bone> n0 = ik_bone_create();
    ik::Ref<ik_bone> n1 = ik_bone_create_child(n0);
    ik::Ref<ik_bone> n2 = ik_bone_create_child(n1);
    ik::Ref<ik_bone> n3 = ik_bone_create_child(n2);
    ik::Ref<ik_bone> n4 = ik_bone_create_child(n2);

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);
    subtree_add_leaf(&subtree, n4);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain_tree), Eq(3));
    EXPECT_THAT(chain_get_base_bone(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_bone(&chain_tree), Eq(n2));

    ASSERT_THAT(chain_child_count(&chain_tree), Eq(2));
    ik_chain* c0 = chain_get_child(&chain_tree, 0);
    ik_chain* c1 = chain_get_child(&chain_tree, 1);

    EXPECT_THAT(chain_child_count(c0), Eq(0));
    EXPECT_THAT(chain_get_base_bone(c0), Eq(n2));
    EXPECT_THAT(chain_get_tip_bone(c0), Eq(n3));

    EXPECT_THAT(chain_child_count(c1), Eq(0));
    EXPECT_THAT(chain_get_base_bone(c1), Eq(n2));
    EXPECT_THAT(chain_get_tip_bone(c1), Eq(n4));
}

TEST_F(NAME, two_arms_with_dead_bones)
{
    /*
     *     n4      n6
     *      \      /
     * e1 -> n3  n5 <- e2
     *        \  /
     *   n8 -- n2 -- n7
     *         |
     *         n1
     *         |
     *         n0 <- a1
     */
    ik::Ref<ik_bone> n0 = ik_bone_create();
    ik::Ref<ik_bone> n1 = ik_bone_create_child(n0);
    ik::Ref<ik_bone> n2 = ik_bone_create_child(n1);
    ik::Ref<ik_bone> n3 = ik_bone_create_child(n2);
    ik::Ref<ik_bone> n4 = ik_bone_create_child(n3);
    ik::Ref<ik_bone> n5 = ik_bone_create_child(n2);
    ik::Ref<ik_bone> n6 = ik_bone_create_child(n5);
    ik::Ref<ik_bone> n7 = ik_bone_create_child(n2);
    ik::Ref<ik_bone> n8 = ik_bone_create_child(n2);

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);
    subtree_add_leaf(&subtree, n5);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain_tree), Eq(3));
    EXPECT_THAT(chain_get_base_bone(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_bone(&chain_tree), Eq(n2));

    ASSERT_THAT(chain_child_count(&chain_tree), Eq(2));
    ik_chain* c0 = chain_get_child(&chain_tree, 0);
    ik_chain* c1 = chain_get_child(&chain_tree, 1);

    EXPECT_THAT(chain_child_count(c0), Eq(0));
    EXPECT_THAT(chain_get_base_bone(c0), Eq(n2));
    EXPECT_THAT(chain_get_tip_bone(c0), Eq(n3));

    EXPECT_THAT(chain_child_count(c1), Eq(0));
    EXPECT_THAT(chain_get_base_bone(c1), Eq(n2));
    EXPECT_THAT(chain_get_tip_bone(c1), Eq(n5));
}

TEST_F(NAME, dead_bones_in_middle_of_chain)
{

}
