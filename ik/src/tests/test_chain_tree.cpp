#include "gmock/gmock.h"
#include "ik/chain_tree.h"
#include "ik/node.h"
#include "ik/subtree.h"

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
    ik_node* node = ik_node_create(ik_guid(0));
    subtree_set_root(&subtree, node);
    subtree_add_leaf(&subtree, node);

    EXPECT_THAT(chain_tree_build(&chain_tree, &subtree), Eq(0));
    IK_DECREF(node);

    EXPECT_THAT(chain_length(&chain_tree), Eq(1));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(node));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(node));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
}

TEST_F(NAME, two_nodes)
{
    ik_node* n0 = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(n0, ik_guid(1));

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n1);

    int result = chain_tree_build(&chain_tree, &subtree);
    EXPECT_THAT(result, Eq(0));
    IK_DECREF(n0);

    EXPECT_THAT(chain_length(&chain_tree), Eq(2));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n1));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
}

TEST_F(NAME, omit_first_and_last)
{
    ik_node* n0 = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(n0, ik_guid(1));
    ik_node* n2 = ik_node_create_child(n1, ik_guid(2));
    ik_node* n3 = ik_node_create_child(n2, ik_guid(3));
    ik_node* n4 = ik_node_create_child(n3, ik_guid(4));

    subtree_set_root(&subtree, n1);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    EXPECT_THAT(result, Eq(0));

    EXPECT_THAT(chain_length(&chain_tree), Eq(3));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n1));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n3));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));

    // Check refcounts
    EXPECT_THAT(IK_REFCOUNT(n0), Eq(1));
    EXPECT_THAT(IK_REFCOUNT(n1), Eq(2));
    EXPECT_THAT(IK_REFCOUNT(n2), Eq(2));
    EXPECT_THAT(IK_REFCOUNT(n3), Eq(2));
    EXPECT_THAT(IK_REFCOUNT(n4), Eq(1));

    IK_DECREF(n0);
}

TEST_F(NAME, ignore_branch_not_part_of_subtree)
{
    ik_node* n0 = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(n0, ik_guid(1));
    ik_node* n2 = ik_node_create_child(n1, ik_guid(2));
    ik_node* n3 = ik_node_create_child(n2, ik_guid(3));
    ik_node* n4 = ik_node_create_child(n2, ik_guid(4));

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);

    int result = chain_tree_build(&chain_tree, &subtree);
    EXPECT_THAT(result, Eq(0));
    IK_DECREF(n0);

    EXPECT_THAT(chain_length(&chain_tree), Eq(4));
    EXPECT_THAT(chain_get_base_node(&chain_tree), Eq(n0));
    EXPECT_THAT(chain_get_tip_node(&chain_tree), Eq(n3));
    EXPECT_THAT(chain_child_count(&chain_tree), Eq(0));
}

TEST_F(NAME, two_arms)
{
    ik_node* n0 = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(n0, ik_guid(1));
    ik_node* n2 = ik_node_create_child(n1, ik_guid(2));
    ik_node* n3 = ik_node_create_child(n2, ik_guid(3));
    ik_node* n4 = ik_node_create_child(n2, ik_guid(4));

    subtree_set_root(&subtree, n0);
    subtree_add_leaf(&subtree, n3);
    subtree_add_leaf(&subtree, n4);

    int result = chain_tree_build(&chain_tree, &subtree);
    EXPECT_THAT(result, Eq(0));
    IK_DECREF(n0);

    EXPECT_THAT(chain_length(&chain_tree), Eq(3));
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
