#include "gmock/gmock.h"
#include "ik/ik.h"
#include "ik/ntf.h"
#include "ik/node.h"
#include "ik/node_data.h"
#include "ik/refcount.h"

#define NAME ntf

using namespace ::testing;

class NAME : public Test
{
public:
    ik_node_t* tree_without_effectors()
    {
        ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9;
        IKAPI.node.create(&tree, IKAPI.to_ptr(0));
        IKAPI.node.create_child(&n1, tree, IKAPI.to_ptr(1));
        IKAPI.node.create_child(&n2, n1, IKAPI.to_ptr(2));
        IKAPI.node.create_child(&n3, n2, IKAPI.to_ptr(3));
        IKAPI.node.create_child(&n4, n3, IKAPI.to_ptr(4));
        IKAPI.node.create_child(&n5, n4, IKAPI.to_ptr(5));
        IKAPI.node.create_child(&n6, n5, IKAPI.to_ptr(6));
        IKAPI.node.create_child(&n7, n3, IKAPI.to_ptr(7));
        IKAPI.node.create_child(&n8, n7, IKAPI.to_ptr(8));
        IKAPI.node.create_child(&n9, n8, IKAPI.to_ptr(9));
        return tree;
    }

    ik_node_t* tree_with_two_effectors()
    {
        ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9;
        IKAPI.node.create(&tree, IKAPI.to_ptr(0));
        IKAPI.node.create_child(&n1, tree, IKAPI.to_ptr(1));
        IKAPI.node.create_child(&n2, n1, IKAPI.to_ptr(2));
        IKAPI.node.create_child(&n3, n2, IKAPI.to_ptr(3));
        IKAPI.node.create_child(&n4, n3, IKAPI.to_ptr(4));
        IKAPI.node.create_child(&n5, n4, IKAPI.to_ptr(5));
        IKAPI.node.create_child(&n6, n5, IKAPI.to_ptr(6));
        IKAPI.node.create_child(&n7, n3, IKAPI.to_ptr(7));
        IKAPI.node.create_child(&n8, n7, IKAPI.to_ptr(8));
        IKAPI.node.create_child(&n9, n8, IKAPI.to_ptr(9));

        ik_effector_t *eff1, *eff2;
        IKAPI.node.create_effector(&eff1, n6);
        IKAPI.node.create_effector(&eff2, n9);

        return tree;
    }

    virtual void SetUp() override
    {

    }

    virtual void TearDown() override
    {
    }
};

TEST_F(NAME, no_action_if_tree_has_no_effectors)
{
    vector_t* ntf_list;
    ik_node_t* tree = tree_without_effectors();
    EXPECT_THAT(ik_ntf_list_from_nodes(&ntf_list, tree), IK_ERR_NO_EFFECTORS_FOUND);
    IKAPI.node.destroy_recursive(tree);
}

TEST_F(NAME, check_refcounts_are_correct)
{
    vector_t* ntf_list;
    ik_node_t* tree = tree_with_two_effectors();

    ASSERT_THAT(ik_ntf_list_from_nodes(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(ntf_list), Eq(1));

    ik_ntf_t* ntf = (ik_ntf_t*)vector_get_element(ntf_list, 0);
    EXPECT_THAT(ntf->node_count, Eq(10));  /* There are 10 nodes in the tree */
    EXPECT_THAT(ntf->node_data->refcount->refs, Eq(11));  /* Each node should hold a reference, plus the ntf structure itself should hold a reference */
    EXPECT_THAT(ntf->node_data, Eq(tree->node_data));

    IKAPI.node.destroy_recursive(tree);
    EXPECT_THAT(ntf->node_data->refcount->refs, Eq(1));  /* ntf structure should still hold a reference */
    ik_ntf_list_destroy(ntf_list);
}

TEST_F(NAME, node_tree_can_be_flattened_multiple_times)
{
    vector_t* ntf_list1;
    vector_t* ntf_list2;
    vector_t* ntf_list3;
    ik_node_t* tree = tree_with_two_effectors();

    ASSERT_THAT(ik_ntf_list_from_nodes(&ntf_list1, tree), IK_OK);
    ASSERT_THAT(ik_ntf_list_from_nodes(&ntf_list2, tree), IK_OK);
    ASSERT_THAT(ik_ntf_list_from_nodes(&ntf_list3, tree), IK_OK);

    ik_ntf_t* ntf1 = (ik_ntf_t*)vector_get_element(ntf_list1, 0);
    ik_ntf_t* ntf2 = (ik_ntf_t*)vector_get_element(ntf_list2, 0);
    ik_ntf_t* ntf3 = (ik_ntf_t*)vector_get_element(ntf_list3, 0);

    EXPECT_THAT(ntf1->node_data, Ne(tree->node_data));
    EXPECT_THAT(ntf2->node_data, Ne(tree->node_data));
    EXPECT_THAT(ntf3->node_data, Eq(tree->node_data));

    EXPECT_THAT(tree->node_data->refcount->refs, Eq(11));
    ik_ntf_list_destroy(ntf_list1);
    EXPECT_THAT(tree->node_data->refcount->refs, Eq(11));
    ik_ntf_list_destroy(ntf_list2);
    EXPECT_THAT(tree->node_data->refcount->refs, Eq(11));
    ik_ntf_list_destroy(ntf_list3);
    EXPECT_THAT(tree->node_data->refcount->refs, Eq(10));

    IKAPI.node.destroy_recursive(tree);
}

TEST_F(NAME, check_indices_are_correct)
{
    vector_t* ntf_list;
    ik_node_t* tree = tree_with_two_effectors();
    ASSERT_THAT(ik_ntf_list_from_nodes(&ntf_list, tree), IK_OK);
    ik_ntf_t* ntf = (ik_ntf_t*)vector_get_element(ntf_list, 0);

    /*
     * Nodes are layed out in memory contiguously with the following
     * offsets (pre-order):
     *
     *  6           9
     *   \         /
     *    5       8
     *     \     /
     *      4   7
     *       \ /
     *        3
     *        |
     *        2
     *        |
     *        1
     *        |
     *        0
     */
    EXPECT_THAT(ntf->indices[0].pre, Eq(0));
    EXPECT_THAT(ntf->indices[1].pre, Eq(1));
    EXPECT_THAT(ntf->indices[2].pre, Eq(2));
    EXPECT_THAT(ntf->indices[3].pre, Eq(3));
    EXPECT_THAT(ntf->indices[4].pre, Eq(4));
    EXPECT_THAT(ntf->indices[5].pre, Eq(5));
    EXPECT_THAT(ntf->indices[6].pre, Eq(6));
    EXPECT_THAT(ntf->indices[7].pre, Eq(7));
    EXPECT_THAT(ntf->indices[8].pre, Eq(8));
    EXPECT_THAT(ntf->indices[9].pre, Eq(9));

    EXPECT_THAT(ntf->indices[0].post, Eq(6));
    EXPECT_THAT(ntf->indices[1].post, Eq(5));
    EXPECT_THAT(ntf->indices[2].post, Eq(4));
    EXPECT_THAT(ntf->indices[3].post, Eq(9));
    EXPECT_THAT(ntf->indices[4].post, Eq(8));
    EXPECT_THAT(ntf->indices[5].post, Eq(7));
    EXPECT_THAT(ntf->indices[6].post, Eq(3));
    EXPECT_THAT(ntf->indices[7].post, Eq(2));
    EXPECT_THAT(ntf->indices[8].post, Eq(1));
    EXPECT_THAT(ntf->indices[9].post, Eq(0));

    EXPECT_THAT(ntf->indices[0].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[1].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[2].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[3].pre_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[4].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[5].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[6].pre_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[7].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[8].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[9].pre_child_count, Eq(0));

    EXPECT_THAT(ntf->indices[0].post_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[1].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[2].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[3].post_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[4].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[5].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[6].post_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[7].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[8].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[9].post_child_count, Eq(1));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_destroy(ntf_list);
}
