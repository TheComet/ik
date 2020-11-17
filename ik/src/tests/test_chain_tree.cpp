#include "gmock/gmock.h"
#include "ik/chain_tree.h"
#include "ik/bone.h"
#include "ik/subtree.h"
#include "ik/cpputils.hpp"

#define NAME chain_tree

using namespace ::testing;

class NAME : public Test
{
public:
    virtual void SetUp()
    {
        subtree_init(&subtree);
        chain_tree_init(&chain);
    }

    virtual void TearDown()
    {
        chain_tree_deinit(&chain);
        subtree_deinit(&subtree);
    }

protected:
    ik_subtree subtree;
    ik_chain chain;
};

TEST_F(NAME, single_bone)
{
    ik::Ref<ik_bone> bone = ik_bone_create(2);
    subtree_set_root(&subtree, bone);
    subtree_add_leaf(&subtree, bone);

    EXPECT_THAT(chain_tree_build(&chain, &subtree), Eq(0));

    EXPECT_THAT(chain_bone_count(&chain), Eq(1));
    EXPECT_THAT(chain_get_base_bone(&chain), Eq(bone));
    EXPECT_THAT(chain_get_tip_bone(&chain), Eq(bone));
    EXPECT_THAT(chain_child_count(&chain), Eq(0));
}

TEST_F(NAME, two_bones)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);

    subtree_set_root(&subtree, b0);
    subtree_add_leaf(&subtree, b1);

    int result = chain_tree_build(&chain, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain), Eq(2));
    EXPECT_THAT(chain_get_base_bone(&chain), Eq(b0));
    EXPECT_THAT(chain_get_tip_bone(&chain), Eq(b1));
    EXPECT_THAT(chain_child_count(&chain), Eq(0));
}

TEST_F(NAME, omit_first_and_last)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2, 2);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b3, 2);

    subtree_set_root(&subtree, b1);
    subtree_add_leaf(&subtree, b3);

    int result = chain_tree_build(&chain, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain), Eq(3));
    EXPECT_THAT(chain_get_base_bone(&chain), Eq(b1));
    EXPECT_THAT(chain_get_tip_bone(&chain), Eq(b3));
    EXPECT_THAT(chain_child_count(&chain), Eq(0));

    // Check refcounts
    EXPECT_THAT(b0.refcount(), Eq(1));
    EXPECT_THAT(b1.refcount(), Eq(3));
    EXPECT_THAT(b2.refcount(), Eq(3));
    EXPECT_THAT(b3.refcount(), Eq(3));
    EXPECT_THAT(b4.refcount(), Eq(2));
}

TEST_F(NAME, ignore_branch_not_part_of_subtree)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2, 2);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b2, 2);

    subtree_set_root(&subtree, b0);
    subtree_add_leaf(&subtree, b3);

    int result = chain_tree_build(&chain, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain), Eq(4));
    EXPECT_THAT(chain_get_base_bone(&chain), Eq(b0));
    EXPECT_THAT(chain_get_tip_bone(&chain), Eq(b3));
    EXPECT_THAT(chain_child_count(&chain), Eq(0));
}

TEST_F(NAME, children_of_leaf_bones_are_not_added_as_dead_bones)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2, 2);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b3, 2);
    ik::Ref<ik_bone> b5 = ik_bone_create_child(b3, 2);

    subtree_set_root(&subtree, b0);
    subtree_add_leaf(&subtree, b3);

    int result = chain_tree_build(&chain, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain), Eq(4));
    EXPECT_THAT(chain_get_base_bone(&chain), Eq(b0));
    EXPECT_THAT(chain_get_tip_bone(&chain), Eq(b3));
    EXPECT_THAT(chain_child_count(&chain), Eq(0));
}

TEST_F(NAME, children_of_base_bone_are_not_added_as_dead_bones)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2, 2);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b0, 2);

    subtree_set_root(&subtree, b0);
    subtree_add_leaf(&subtree, b3);

    int result = chain_tree_build(&chain, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain), Eq(4));
    EXPECT_THAT(chain_get_base_bone(&chain), Eq(b0));
    EXPECT_THAT(chain_get_tip_bone(&chain), Eq(b3));
    EXPECT_THAT(chain_child_count(&chain), Eq(0));
}

TEST_F(NAME, two_arms)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2, 2);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b2, 2);

    subtree_set_root(&subtree, b0);
    subtree_add_leaf(&subtree, b3);
    subtree_add_leaf(&subtree, b4);

    int result = chain_tree_build(&chain, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain), Eq(3));
    EXPECT_THAT(chain_get_base_bone(&chain), Eq(b0));
    EXPECT_THAT(chain_get_tip_bone(&chain), Eq(b2));

    ASSERT_THAT(chain_child_count(&chain), Eq(2));
    ik_chain* c0 = chain_get_child(&chain, 0);
    ik_chain* c1 = chain_get_child(&chain, 1);

    EXPECT_THAT(chain_child_count(c0), Eq(0));
    EXPECT_THAT(chain_bone_count(c0), Eq(1));
    EXPECT_THAT(chain_get_base_bone(c0), Eq(b3));
    EXPECT_THAT(chain_get_tip_bone(c0), Eq(b3));

    EXPECT_THAT(chain_child_count(c1), Eq(0));
    EXPECT_THAT(chain_bone_count(c1), Eq(1));
    EXPECT_THAT(chain_get_base_bone(c1), Eq(b4));
    EXPECT_THAT(chain_get_tip_bone(c1), Eq(b4));
}

TEST_F(NAME, two_arms_with_dead_bones)
{
    /*
     *     b4      b6
     *      \      /
     * e1 -> b3  b5 <- e2
     *        \  /
     *   b8 -- b2 -- b7
     *         |
     *         b1
     *         |
     *         b0 <- a1
     */
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2, 2);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b3, 2);
    ik::Ref<ik_bone> b5 = ik_bone_create_child(b2, 2);
    ik::Ref<ik_bone> b6 = ik_bone_create_child(b5, 2);
    ik::Ref<ik_bone> b7 = ik_bone_create_child(b2, 2);
    ik::Ref<ik_bone> b8 = ik_bone_create_child(b2, 2);

    subtree_set_root(&subtree, b0);
    subtree_add_leaf(&subtree, b3);
    subtree_add_leaf(&subtree, b5);

    int result = chain_tree_build(&chain, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_bone_count(&chain), Eq(3));
    EXPECT_THAT(chain_get_base_bone(&chain), Eq(b0));
    EXPECT_THAT(chain_get_tip_bone(&chain), Eq(b2));

    ASSERT_THAT(chain_child_count(&chain), Eq(2));
    ik_chain* c0 = chain_get_child(&chain, 0);
    ik_chain* c1 = chain_get_child(&chain, 1);

    EXPECT_THAT(chain_child_count(c0), Eq(0));
    EXPECT_THAT(chain_get_base_bone(c0), Eq(b3));
    EXPECT_THAT(chain_get_tip_bone(c0), Eq(b3));

    EXPECT_THAT(chain_child_count(c1), Eq(0));
    EXPECT_THAT(chain_get_base_bone(c1), Eq(b5));
    EXPECT_THAT(chain_get_tip_bone(c1), Eq(b5));
}

TEST_F(NAME, dead_bones_in_middle_of_chain)
{

}
