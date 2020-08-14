#include "gmock/gmock.h"
#include "ik/chain_tree.h"
#include "ik/node.h"
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

TEST_F(NAME, single_node)
{
    ik::Ref<ik_node> node = ik_node_create(ik_guid(0));
    subtree_set_root(&subtree, node);
    subtree_add_leaf(&subtree, node);

    EXPECT_THAT(chain_tree_build(&chain_tree, &subtree), Eq(0));

    EXPECT_THAT(chain_node_count(&chain_tree), Eq(1));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(node));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(node));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
}

TEST_F(NAME, two_nodes)
{
    ik::Ref<ik_node> n0 = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> n1 = ik_node_create_child(n0, ik_guid(1));

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n1);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_node_count(&chain_tree), Eq(2));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n1));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
    EXPECT_THAT(chain_dead_node_count(&chain_tree), Eq(0));
}

TEST_F(NAME, omit_first_and_last)
{
    ik::Ref<ik_node> n0 = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> n1 = ik_node_create_child(n0, ik_guid(1));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(2));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(3));
    ik::Ref<ik_node> n4 = ik_node_create_child(n3, ik_guid(4));

    subtree_set_root(&subtree, n1);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_node_count(&chain_tree), Eq(3));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n1));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n3));
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
    ik::Ref<ik_node> n0 = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> n1 = ik_node_create_child(n0, ik_guid(1));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(2));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(3));
    ik::Ref<ik_node> n4 = ik_node_create_child(n2, ik_guid(4));

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_node_count(&chain_tree), Eq(4));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n3));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
    EXPECT_THAT(chain_dead_node_count(&chain_tree), Eq(0));
}

TEST_F(NAME, children_of_leaf_nodes_are_not_added_as_dead_nodes)
{
    ik::Ref<ik_node> n0 = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> n1 = ik_node_create_child(n0, ik_guid(1));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(2));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(3));
    ik::Ref<ik_node> n4 = ik_node_create_child(n3, ik_guid(4));
    ik::Ref<ik_node> n5 = ik_node_create_child(n3, ik_guid(5));

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_node_count(&chain_tree), Eq(4));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n3));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
    EXPECT_THAT(chain_dead_node_count(&chain_tree), Eq(0));
}

TEST_F(NAME, children_of_base_node_are_not_added_as_dead_nodes)
{
    ik::Ref<ik_node> n0 = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> n1 = ik_node_create_child(n0, ik_guid(1));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(2));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(3));
    ik::Ref<ik_node> n4 = ik_node_create_child(n0, ik_guid(3));

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_node_count(&chain_tree), Eq(4));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n3));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
    EXPECT_THAT(chain_dead_node_count(&chain_tree), Eq(0));
}

TEST_F(NAME, two_arms)
{
    ik::Ref<ik_node> n0 = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> n1 = ik_node_create_child(n0, ik_guid(1));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(2));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(3));
    ik::Ref<ik_node> n4 = ik_node_create_child(n2, ik_guid(4));

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);
    subtree_add_leaf(&subtree, n4);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_node_count(&chain_tree), Eq(3));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n2));

    ASSERT_THAT(chain_child_count(&chain_tree), Eq(2));
    ik_chain* c0 = chain_get_child(&chain_tree, 0);
    ik_chain* c1 = chain_get_child(&chain_tree, 1);

    EXPECT_THAT(chain_child_count(c0), Eq(0));
    EXPECT_THAT(chain_get_base_node(c0), Eq(n2));
    EXPECT_THAT(chain_get_tip_node(c0), Eq(n3));

    EXPECT_THAT(chain_child_count(c1), Eq(0));
    EXPECT_THAT(chain_get_base_node(c1), Eq(n2));
    EXPECT_THAT(chain_get_tip_node(c1), Eq(n4));
}

TEST_F(NAME, two_arms_with_dead_nodes)
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
    ik::Ref<ik_node> n0 = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> n1 = ik_node_create_child(n0, ik_guid(1));
    ik::Ref<ik_node> n2 = ik_node_create_child(n1, ik_guid(2));
    ik::Ref<ik_node> n3 = ik_node_create_child(n2, ik_guid(3));
    ik::Ref<ik_node> n4 = ik_node_create_child(n3, ik_guid(4));
    ik::Ref<ik_node> n5 = ik_node_create_child(n2, ik_guid(5));
    ik::Ref<ik_node> n6 = ik_node_create_child(n5, ik_guid(6));
    ik::Ref<ik_node> n7 = ik_node_create_child(n2, ik_guid(7));
    ik::Ref<ik_node> n8 = ik_node_create_child(n2, ik_guid(8));

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);
    subtree_add_leaf(&subtree, n5);

    int result = chain_tree_build(&chain_tree, &subtree);
    ASSERT_THAT(result, Eq(0));

    EXPECT_THAT(chain_node_count(&chain_tree), Eq(3));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n2));

    ASSERT_THAT(chain_dead_node_count(&chain_tree), Eq(2));
    EXPECT_THAT(chain_get_dead_node(&chain_tree, 0), Eq(n7));
    EXPECT_THAT(chain_get_dead_node(&chain_tree, 1), Eq(n8));

    ASSERT_THAT(chain_child_count(&chain_tree), Eq(2));
    ik_chain* c0 = chain_get_child(&chain_tree, 0);
    ik_chain* c1 = chain_get_child(&chain_tree, 1);

    EXPECT_THAT(chain_child_count(c0), Eq(0));
    EXPECT_THAT(chain_get_base_node(c0), Eq(n2));
    EXPECT_THAT(chain_get_tip_node(c0), Eq(n3));
    EXPECT_THAT(chain_dead_node_count(c0), Eq(0));

    EXPECT_THAT(chain_child_count(c1), Eq(0));
    EXPECT_THAT(chain_get_base_node(c1), Eq(n2));
    EXPECT_THAT(chain_get_tip_node(c1), Eq(n5));
    EXPECT_THAT(chain_dead_node_count(c1), Eq(0));
}
