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
        ik_node_t *tree, *n6, *n9;
        tree = tree_without_effectors();
        IKAPI.node.find_child(&n6, tree, IKAPI.to_ptr(6));
        IKAPI.node.find_child(&n9, tree, IKAPI.to_ptr(9));

        ik_effector_t *eff1, *eff2;
        IKAPI.node.create_effector(&eff1, n6);
        IKAPI.node.create_effector(&eff2, n9);

        return tree;
    }

    ik_node_t* tree_llr()
    {

        /*
         * e1 -> 6       8 <- e2
         *        \     /
         *         5   7
         *          \ /
         *    b2 ->  4      10 <- e3
         *            \     /
         *             3   9
         *              \ /
         *               2  <- b1
         *               |
         *               1
         *               |
         *               0
         */
        ik_node_t *tree, *b1, *b2, *e1, *e2, *e3;
        IKAPI.node.create(&tree, IKAPI.to_ptr(0));
        IKAPI.node.create_child(&b1, tree, IKAPI.to_ptr(1));
        IKAPI.node.create_child(&b1, b1, IKAPI.to_ptr(2));
        IKAPI.node.create_child(&b2, b1, IKAPI.to_ptr(3));
        IKAPI.node.create_child(&b2, b2, IKAPI.to_ptr(4));
        IKAPI.node.create_child(&e1, b2, IKAPI.to_ptr(5));
        IKAPI.node.create_child(&e1, e1, IKAPI.to_ptr(6));
        IKAPI.node.create_child(&e2, b2, IKAPI.to_ptr(7));
        IKAPI.node.create_child(&e2, e2, IKAPI.to_ptr(8));
        IKAPI.node.create_child(&e3, b1, IKAPI.to_ptr(9));
        IKAPI.node.create_child(&e3, e3, IKAPI.to_ptr(10));

        ik_effector_t *eff1, *eff2, *eff3;
        IKAPI.node.create_effector(&eff1, e1);
        IKAPI.node.create_effector(&eff2, e2);
        IKAPI.node.create_effector(&eff3, e3);

        return tree;
    }

    ik_node_t* tree_lrr()
    {
        /*
         *     e2 -> 8       10 <- e3
         *            \     /
         *             7   9
         *              \ /
         * e1 -> 4       6 <- b2
         *        \     /
         *         3   5
         *          \ /
         *           2  <- b1
         *           |
         *           1
         *           |
         *           0
         */
        ik_node_t *tree, *b1, *b2, *e1, *e2, *e3;
        IKAPI.node.create(&tree, IKAPI.to_ptr(0));
        IKAPI.node.create_child(&b1, tree, IKAPI.to_ptr(1));
        IKAPI.node.create_child(&b1, b1, IKAPI.to_ptr(2));
        IKAPI.node.create_child(&e1, b1, IKAPI.to_ptr(3));
        IKAPI.node.create_child(&e1, e1, IKAPI.to_ptr(4));
        IKAPI.node.create_child(&b2, b1, IKAPI.to_ptr(5));
        IKAPI.node.create_child(&b2, b2, IKAPI.to_ptr(6));
        IKAPI.node.create_child(&e2, b2, IKAPI.to_ptr(7));
        IKAPI.node.create_child(&e2, e2, IKAPI.to_ptr(8));
        IKAPI.node.create_child(&e3, b2, IKAPI.to_ptr(9));
        IKAPI.node.create_child(&e3, e3, IKAPI.to_ptr(10));

        ik_effector_t *eff1, *eff2, *eff3;
        IKAPI.node.create_effector(&eff1, e1);
        IKAPI.node.create_effector(&eff2, e2);
        IKAPI.node.create_effector(&eff3, e3);

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
    ik_node_t* tree = tree_without_effectors();
    vector_t ntf_list;
    ik_ntf_list_construct(&ntf_list);
    EXPECT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_ERR_NO_EFFECTORS_FOUND);
    IKAPI.node.destroy_recursive(tree);
}

TEST_F(NAME, check_refcounts_are_correct)
{
    vector_t ntf_list;
    ik_node_t* tree = tree_with_two_effectors();

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(1));

    ik_ntf_t* ntf = (ik_ntf_t*)vector_get_element(&ntf_list, 0);
    EXPECT_THAT(ntf->node_count, Eq(10));  /* There are 10 nodes in the tree */
    EXPECT_THAT(ntf->node_data->refcount->refs, Eq(11));  /* Each node should hold a reference, plus the ntf structure itself should hold a reference */
    EXPECT_THAT(ntf->node_data, Eq(tree->node_data));

    IKAPI.node.destroy_recursive(tree);
    EXPECT_THAT(ntf->node_data->refcount->refs, Eq(1));  /* ntf structure should still hold a reference */
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, node_tree_can_be_flattened_multiple_times)
{
    vector_t ntf_list1;
    vector_t ntf_list2;
    vector_t ntf_list3;
    ik_node_t* tree = tree_with_two_effectors();

    ik_ntf_list_construct(&ntf_list1);
    ik_ntf_list_construct(&ntf_list2);
    ik_ntf_list_construct(&ntf_list3);

    ASSERT_THAT(ik_ntf_list_fill(&ntf_list1, tree), IK_OK);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list2, tree), IK_OK);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list3, tree), IK_OK);

    ik_ntf_t* ntf1 = (ik_ntf_t*)vector_get_element(&ntf_list1, 0);
    ik_ntf_t* ntf2 = (ik_ntf_t*)vector_get_element(&ntf_list2, 0);
    ik_ntf_t* ntf3 = (ik_ntf_t*)vector_get_element(&ntf_list3, 0);

    /* The newly created flattened node data should be the one pointing to
     * the original tree node data */
    EXPECT_THAT(ntf1->node_data, Ne(tree->node_data));
    EXPECT_THAT(ntf2->node_data, Ne(tree->node_data));
    EXPECT_THAT(ntf3->node_data, Eq(tree->node_data));

    EXPECT_THAT(tree->node_data->refcount->refs, Eq(11));
    ik_ntf_list_clear(&ntf_list1);
    EXPECT_THAT(tree->node_data->refcount->refs, Eq(11));
    ik_ntf_list_clear(&ntf_list2);
    EXPECT_THAT(tree->node_data->refcount->refs, Eq(11));
    ik_ntf_list_clear(&ntf_list3);
    EXPECT_THAT(tree->node_data->refcount->refs, Eq(10));

    IKAPI.node.destroy_recursive(tree);
}

TEST_F(NAME, check_indices_are_correct)
{
    vector_t ntf_list;
    ik_node_t* tree = tree_with_two_effectors();

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(1));
    ik_ntf_t* ntf = (ik_ntf_t*)vector_get_element(&ntf_list, 0);

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
    EXPECT_THAT(ntf->indices[0].pre_node, Eq(0));
    EXPECT_THAT(ntf->indices[1].pre_node, Eq(1));
    EXPECT_THAT(ntf->indices[2].pre_node, Eq(2));
    EXPECT_THAT(ntf->indices[3].pre_node, Eq(3));
    EXPECT_THAT(ntf->indices[4].pre_node, Eq(4));
    EXPECT_THAT(ntf->indices[5].pre_node, Eq(5));
    EXPECT_THAT(ntf->indices[6].pre_node, Eq(6));
    EXPECT_THAT(ntf->indices[7].pre_node, Eq(7));
    EXPECT_THAT(ntf->indices[8].pre_node, Eq(8));
    EXPECT_THAT(ntf->indices[9].pre_node, Eq(9));

    EXPECT_THAT(ntf->indices[0].post_node, Eq(6));
    EXPECT_THAT(ntf->indices[1].post_node, Eq(5));
    EXPECT_THAT(ntf->indices[2].post_node, Eq(4));
    EXPECT_THAT(ntf->indices[3].post_node, Eq(9));
    EXPECT_THAT(ntf->indices[4].post_node, Eq(8));
    EXPECT_THAT(ntf->indices[5].post_node, Eq(7));
    EXPECT_THAT(ntf->indices[6].post_node, Eq(3));
    EXPECT_THAT(ntf->indices[7].post_node, Eq(2));
    EXPECT_THAT(ntf->indices[8].post_node, Eq(1));
    EXPECT_THAT(ntf->indices[9].post_node, Eq(0));

    EXPECT_THAT(ntf->indices[0].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[1].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[2].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[3].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[4].pre_base, Eq(3));
    EXPECT_THAT(ntf->indices[5].pre_base, Eq(3));
    EXPECT_THAT(ntf->indices[6].pre_base, Eq(3));
    EXPECT_THAT(ntf->indices[7].pre_base, Eq(3));
    EXPECT_THAT(ntf->indices[8].pre_base, Eq(3));
    EXPECT_THAT(ntf->indices[9].pre_base, Eq(3));

    EXPECT_THAT(ntf->indices[0].post_base, Eq(3));
    EXPECT_THAT(ntf->indices[1].post_base, Eq(3));
    EXPECT_THAT(ntf->indices[2].post_base, Eq(3));
    EXPECT_THAT(ntf->indices[3].post_base, Eq(3));
    EXPECT_THAT(ntf->indices[4].post_base, Eq(3));
    EXPECT_THAT(ntf->indices[5].post_base, Eq(3));
    EXPECT_THAT(ntf->indices[6].post_base, Eq(0));
    EXPECT_THAT(ntf->indices[7].post_base, Eq(0));
    EXPECT_THAT(ntf->indices[8].post_base, Eq(0));
    EXPECT_THAT(ntf->indices[9].post_base, Eq(0));

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
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, check_if_indices_are_correct_llr)
{

    vector_t ntf_list;
    ik_node_t* tree = tree_llr();

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ik_ntf_t* ntf = (ik_ntf_t*)vector_get_element(&ntf_list, 0);

    /*
     * e1 -> 6       8 <- e2
     *        \     /
     *         5   7
     *          \ /
     *    b2 ->  4      10 <- e3
     *            \     /
     *             3   9
     *              \ /
     *               2  <- b1
     *               |
     *               1
     *               |
     *               0
     */
    EXPECT_THAT(ntf->indices[0 ].pre_node, Eq(0));
    EXPECT_THAT(ntf->indices[1 ].pre_node, Eq(1));
    EXPECT_THAT(ntf->indices[2 ].pre_node, Eq(2));
    EXPECT_THAT(ntf->indices[3 ].pre_node, Eq(3));
    EXPECT_THAT(ntf->indices[4 ].pre_node, Eq(4));
    EXPECT_THAT(ntf->indices[5 ].pre_node, Eq(5));
    EXPECT_THAT(ntf->indices[6 ].pre_node, Eq(6));
    EXPECT_THAT(ntf->indices[7 ].pre_node, Eq(7));
    EXPECT_THAT(ntf->indices[8 ].pre_node, Eq(8));
    EXPECT_THAT(ntf->indices[9 ].pre_node, Eq(9));
    EXPECT_THAT(ntf->indices[10].pre_node, Eq(10));

    EXPECT_THAT(ntf->indices[0 ].post_node, Eq(6));
    EXPECT_THAT(ntf->indices[1 ].post_node, Eq(5));
    EXPECT_THAT(ntf->indices[2 ].post_node, Eq(8));
    EXPECT_THAT(ntf->indices[3 ].post_node, Eq(7));
    EXPECT_THAT(ntf->indices[4 ].post_node, Eq(4));
    EXPECT_THAT(ntf->indices[5 ].post_node, Eq(3));
    EXPECT_THAT(ntf->indices[6 ].post_node, Eq(10));
    EXPECT_THAT(ntf->indices[7 ].post_node, Eq(9));
    EXPECT_THAT(ntf->indices[8 ].post_node, Eq(2));
    EXPECT_THAT(ntf->indices[9 ].post_node, Eq(1));
    EXPECT_THAT(ntf->indices[10].post_node, Eq(0));

    EXPECT_THAT(ntf->indices[0 ].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[1 ].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[2 ].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[3 ].pre_base, Eq(2));
    EXPECT_THAT(ntf->indices[4 ].pre_base, Eq(2));
    EXPECT_THAT(ntf->indices[5 ].pre_base, Eq(4));
    EXPECT_THAT(ntf->indices[6 ].pre_base, Eq(4));
    EXPECT_THAT(ntf->indices[7 ].pre_base, Eq(4));
    EXPECT_THAT(ntf->indices[8 ].pre_base, Eq(4));
    EXPECT_THAT(ntf->indices[9 ].pre_base, Eq(2));
    EXPECT_THAT(ntf->indices[10].pre_base, Eq(2));

    EXPECT_THAT(ntf->indices[0 ].post_base, Eq(4));
    EXPECT_THAT(ntf->indices[1 ].post_base, Eq(4));
    EXPECT_THAT(ntf->indices[2 ].post_base, Eq(4));
    EXPECT_THAT(ntf->indices[3 ].post_base, Eq(4));
    EXPECT_THAT(ntf->indices[4 ].post_base, Eq(2));
    EXPECT_THAT(ntf->indices[5 ].post_base, Eq(2));
    EXPECT_THAT(ntf->indices[6 ].post_base, Eq(2));
    EXPECT_THAT(ntf->indices[7 ].post_base, Eq(2));
    EXPECT_THAT(ntf->indices[8 ].post_base, Eq(0));
    EXPECT_THAT(ntf->indices[9 ].post_base, Eq(0));
    EXPECT_THAT(ntf->indices[10].post_base, Eq(0));

    EXPECT_THAT(ntf->indices[0 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[1 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[2 ].pre_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[3 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[4 ].pre_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[5 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[6 ].pre_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[7 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[8 ].pre_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[9 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[10].pre_child_count, Eq(0));

    EXPECT_THAT(ntf->indices[0 ].post_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[1 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[2 ].post_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[3 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[4 ].post_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[5 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[6 ].post_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[7 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[8 ].post_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[9 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[10].post_child_count, Eq(1));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, check_if_indices_are_correct_lrr)
{
    vector_t ntf_list;
    ik_node_t* tree = tree_lrr();
    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ik_ntf_t* ntf = (ik_ntf_t*)vector_get_element(&ntf_list, 0);

    /*
     *     e2 -> 8       10 <- e3
     *            \     /
     *             7   9
     *              \ /
     * e1 -> 4       6 <- b2
     *        \     /
     *         3   5
     *          \ /
     *           2  <- b1
     *           |
     *           1
     *           |
     *           0
     */
    EXPECT_THAT(ntf->indices[0 ].pre_node, Eq(0));
    EXPECT_THAT(ntf->indices[1 ].pre_node, Eq(1));
    EXPECT_THAT(ntf->indices[2 ].pre_node, Eq(2));
    EXPECT_THAT(ntf->indices[3 ].pre_node, Eq(3));
    EXPECT_THAT(ntf->indices[4 ].pre_node, Eq(4));
    EXPECT_THAT(ntf->indices[5 ].pre_node, Eq(5));
    EXPECT_THAT(ntf->indices[6 ].pre_node, Eq(6));
    EXPECT_THAT(ntf->indices[7 ].pre_node, Eq(7));
    EXPECT_THAT(ntf->indices[8 ].pre_node, Eq(8));
    EXPECT_THAT(ntf->indices[9 ].pre_node, Eq(9));
    EXPECT_THAT(ntf->indices[10].pre_node, Eq(10));

    EXPECT_THAT(ntf->indices[0 ].post_node, Eq(4));
    EXPECT_THAT(ntf->indices[1 ].post_node, Eq(3));
    EXPECT_THAT(ntf->indices[2 ].post_node, Eq(8));
    EXPECT_THAT(ntf->indices[3 ].post_node, Eq(7));
    EXPECT_THAT(ntf->indices[4 ].post_node, Eq(10));
    EXPECT_THAT(ntf->indices[5 ].post_node, Eq(9));
    EXPECT_THAT(ntf->indices[6 ].post_node, Eq(6));
    EXPECT_THAT(ntf->indices[7 ].post_node, Eq(5));
    EXPECT_THAT(ntf->indices[8 ].post_node, Eq(2));
    EXPECT_THAT(ntf->indices[9 ].post_node, Eq(1));
    EXPECT_THAT(ntf->indices[10].post_node, Eq(0));

    EXPECT_THAT(ntf->indices[0 ].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[1 ].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[2 ].pre_base, Eq(0));
    EXPECT_THAT(ntf->indices[3 ].pre_base, Eq(2));
    EXPECT_THAT(ntf->indices[4 ].pre_base, Eq(2));
    EXPECT_THAT(ntf->indices[5 ].pre_base, Eq(2));
    EXPECT_THAT(ntf->indices[6 ].pre_base, Eq(2));
    EXPECT_THAT(ntf->indices[7 ].pre_base, Eq(6));
    EXPECT_THAT(ntf->indices[8 ].pre_base, Eq(6));
    EXPECT_THAT(ntf->indices[9 ].pre_base, Eq(6));
    EXPECT_THAT(ntf->indices[10].pre_base, Eq(6));

    EXPECT_THAT(ntf->indices[0 ].post_base, Eq(2));
    EXPECT_THAT(ntf->indices[1 ].post_base, Eq(2));
    EXPECT_THAT(ntf->indices[2 ].post_base, Eq(6));
    EXPECT_THAT(ntf->indices[3 ].post_base, Eq(6));
    EXPECT_THAT(ntf->indices[4 ].post_base, Eq(6));
    EXPECT_THAT(ntf->indices[5 ].post_base, Eq(6));
    EXPECT_THAT(ntf->indices[6 ].post_base, Eq(2));
    EXPECT_THAT(ntf->indices[7 ].post_base, Eq(2));
    EXPECT_THAT(ntf->indices[8 ].post_base, Eq(0));
    EXPECT_THAT(ntf->indices[9 ].post_base, Eq(0));
    EXPECT_THAT(ntf->indices[10].post_base, Eq(0));

    EXPECT_THAT(ntf->indices[0 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[1 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[2 ].pre_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[3 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[4 ].pre_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[5 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[6 ].pre_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[7 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[8 ].pre_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[9 ].pre_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[10].pre_child_count, Eq(0));

    EXPECT_THAT(ntf->indices[0 ].post_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[1 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[2 ].post_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[3 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[4 ].post_child_count, Eq(0));
    EXPECT_THAT(ntf->indices[5 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[6 ].post_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[7 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[8 ].post_child_count, Eq(2));
    EXPECT_THAT(ntf->indices[9 ].post_child_count, Eq(1));
    EXPECT_THAT(ntf->indices[10].post_child_count, Eq(1));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, ignore_effector_on_root_node)
{
    vector_t ntf_list;
    ik_node_t *tree, *n1, *n2;

    IKAPI.node.create(&tree, IKAPI.to_ptr(0));
    IKAPI.node.create_child(&n1, tree, IKAPI.to_ptr(1));
    IKAPI.node.create_child(&n2,  n1,  IKAPI.to_ptr(2));

    ik_effector_t *e1, *e2;
    IKAPI.node.create_effector(&e1, tree);
    IKAPI.node.create_effector(&e2, n2);

    /*
     *  2 <- e2
     *  |
     *  1
     *  |
     *  0 <- e1
     *
     */

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(1));

    ik_ntf_t* ntf = (ik_ntf_t*)vector_get_element(&ntf_list, 0);
    EXPECT_THAT(ntf->node_count, Eq(3));
    EXPECT_THAT(NTF_PRE_NODE(ntf, 0), Eq(tree->node_data));
    EXPECT_THAT(NTF_POST_NODE(ntf, 0), Eq(n2->node_data));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, split_trees_on_effectors)
{
    vector_t ntf_list;
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6;

    IKAPI.node.create(&tree, IKAPI.to_ptr(0));
    IKAPI.node.create_child(&n1, tree, IKAPI.to_ptr(1));
    IKAPI.node.create_child(&n2,  n1,  IKAPI.to_ptr(2));
    IKAPI.node.create_child(&n3,  n2,  IKAPI.to_ptr(3));
    IKAPI.node.create_child(&n4,  n3,  IKAPI.to_ptr(4));
    IKAPI.node.create_child(&n5,  n4,  IKAPI.to_ptr(5));
    IKAPI.node.create_child(&n6,  n5,  IKAPI.to_ptr(6));

    ik_effector_t *e1, *e2, *e3;
    IKAPI.node.create_effector(&e1, n2);
    IKAPI.node.create_effector(&e2, n3);
    IKAPI.node.create_effector(&e3, n5);

    /*
     *       6
     *       |
     *       5 <- e3
     *       |
     *       4
     *       |
     *       3 <- e2
     *       |
     *       2 <- e1
     *       |
     *       1
     *       |
     *       0
     *
     */

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(3));

    ik_ntf_t *ntf1, *ntf2, *ntf3;
    ntf1 = (ik_ntf_t*)vector_get_element(&ntf_list, 0);
    ntf2 = (ik_ntf_t*)vector_get_element(&ntf_list, 1);
    ntf3 = (ik_ntf_t*)vector_get_element(&ntf_list, 2);

    EXPECT_THAT(NTF_PRE_BASE(ntf1, 0), Eq(tree->node_data));
    EXPECT_THAT(ntf1->node_count, Eq(3));  // 0,1,2
    EXPECT_THAT(NTF_PRE_BASE(ntf2, 0), Eq(n2->node_data));
    EXPECT_THAT(ntf2->node_count, Eq(2));  // 2,3,4
    EXPECT_THAT(NTF_PRE_BASE(ntf3, 0), Eq(n3->node_data));
    EXPECT_THAT(ntf3->node_count, Eq(3));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, split_trees_on_effectors_with_chain_lengths)
{
    vector_t ntf_list;
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6;

    IKAPI.node.create(&tree, IKAPI.to_ptr(0));
    IKAPI.node.create_child(&n1, tree, IKAPI.to_ptr(1));
    IKAPI.node.create_child(&n2,  n1,  IKAPI.to_ptr(2));
    IKAPI.node.create_child(&n3,  n2,  IKAPI.to_ptr(3));
    IKAPI.node.create_child(&n4,  n3,  IKAPI.to_ptr(4));
    IKAPI.node.create_child(&n5,  n4,  IKAPI.to_ptr(5));
    IKAPI.node.create_child(&n6,  n5,  IKAPI.to_ptr(6));

    ik_effector_t *e1, *e2, *e3;
    IKAPI.node.create_effector(&e1, n2);
    IKAPI.node.create_effector(&e2, n3);
    IKAPI.node.create_effector(&e3, n5);
    IKAPI.effector.set_chain_length(e3, 1);

    /*
     *       6
     *       |
     *       5 <- e3 (chain length = 1)
     *       |
     *       4
     *       |
     *       3 <- e2
     *       |
     *       2 <- e1
     *       |
     *       1
     *       |
     *       0
     *
     */

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(3));

    ik_ntf_t *ntf1, *ntf2, *ntf3;
    ntf1 = (ik_ntf_t*)vector_get_element(&ntf_list, 0);
    ntf2 = (ik_ntf_t*)vector_get_element(&ntf_list, 1);
    ntf3 = (ik_ntf_t*)vector_get_element(&ntf_list, 2);

    EXPECT_THAT(NTF_PRE_BASE(ntf1, 0), Eq(tree->node_data));
    EXPECT_THAT(ntf1->node_count, Eq(3));  // 0,1,2
    EXPECT_THAT(NTF_PRE_BASE(ntf2, 0), Eq(n2->node_data));
    EXPECT_THAT(ntf2->node_count, Eq(2));  // 2,3,4
    EXPECT_THAT(NTF_PRE_BASE(ntf3, 0), Eq(n4->node_data));
    EXPECT_THAT(ntf3->node_count, Eq(2));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, ignore_effector_on_root_node_with_dead_nodes)
{
    vector_t ntf_list;
    ik_node_t *tree, *n1, *n2, *n3, *n4;

    IKAPI.node.create(&tree, IKAPI.to_ptr(0));
    IKAPI.node.create_child(&n1, tree, IKAPI.to_ptr(1));
    IKAPI.node.create_child(&n2, n1,  IKAPI.to_ptr(2));
    IKAPI.node.create_child(&n3, tree,  IKAPI.to_ptr(3));
    IKAPI.node.create_child(&n4, n1,  IKAPI.to_ptr(4));

    ik_effector_t *e1, *e2;
    IKAPI.node.create_effector(&e1, tree);
    IKAPI.node.create_effector(&e2, n2);

    /*
     *  4 2 <- e2
     *   \|
     *  3 1
     *   \|
     *    0 <- e1
     *
     */

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(1));

    ik_ntf_t* ntf = (ik_ntf_t*)vector_get_element(&ntf_list, 0);
    EXPECT_THAT(ntf->node_count, Eq(3));
    EXPECT_THAT(NTF_PRE_NODE(ntf, 0), Eq(tree->node_data));
    EXPECT_THAT(NTF_POST_NODE(ntf, 0), Eq(n2->node_data));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, split_trees_on_effectors_with_dead_nodes)
{
    vector_t ntf_list;
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6, *dead;

    IKAPI.node.create(&tree, IKAPI.to_ptr(0));
    IKAPI.node.create_child(&n1, tree, IKAPI.to_ptr(1));
    IKAPI.node.create_child(&n2, n1,  IKAPI.to_ptr(2));
    IKAPI.node.create_child(&n3, n2,  IKAPI.to_ptr(3));
    IKAPI.node.create_child(&n4, n3,  IKAPI.to_ptr(4));
    IKAPI.node.create_child(&n5, n4,  IKAPI.to_ptr(5));
    IKAPI.node.create_child(&n6, n5,  IKAPI.to_ptr(6));
    IKAPI.node.create_child(&dead, tree,  IKAPI.to_ptr(7));
    IKAPI.node.create_child(&dead, n1,  IKAPI.to_ptr(8));
    IKAPI.node.create_child(&dead, n2,  IKAPI.to_ptr(9));
    IKAPI.node.create_child(&dead, n3,  IKAPI.to_ptr(10));
    IKAPI.node.create_child(&dead, n4,  IKAPI.to_ptr(11));
    IKAPI.node.create_child(&dead, n5,  IKAPI.to_ptr(12));
    IKAPI.node.create_child(&dead, n6,  IKAPI.to_ptr(13));

    ik_effector_t *e1, *e2, *e3;
    IKAPI.node.create_effector(&e1, n2);
    IKAPI.node.create_effector(&e2, n3);
    IKAPI.node.create_effector(&e3, n5);

    /*
     *   13--6
     *       |
     *   12--5 <- e3
     *       |
     *   11--4
     *       |
     *   10--3 <- e2
     *       |
     *    9--2 <- e1
     *       |
     *    8--1
     *       |
     *    7--0
     *
     */

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(3));

    ik_ntf_t *ntf1, *ntf2, *ntf3;
    ntf1 = (ik_ntf_t*)vector_get_element(&ntf_list, 0);
    ntf2 = (ik_ntf_t*)vector_get_element(&ntf_list, 1);
    ntf3 = (ik_ntf_t*)vector_get_element(&ntf_list, 2);

    EXPECT_THAT(NTF_PRE_BASE(ntf1, 0), Eq(tree->node_data));
    EXPECT_THAT(ntf1->node_count, Eq(3));  // 0,1,2
    EXPECT_THAT(NTF_PRE_BASE(ntf2, 0), Eq(n2->node_data));
    EXPECT_THAT(ntf2->node_count, Eq(2));  // 2,3,4
    EXPECT_THAT(NTF_PRE_BASE(ntf3, 0), Eq(n3->node_data));
    EXPECT_THAT(ntf3->node_count, Eq(3));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, split_trees_on_effectors_with_chain_lengths_with_dead_nodes)
{
    vector_t ntf_list;
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6, *dead;

    IKAPI.node.create(&tree, IKAPI.to_ptr(0));
    IKAPI.node.create_child(&n1, tree, IKAPI.to_ptr(1));
    IKAPI.node.create_child(&n2, n1,  IKAPI.to_ptr(2));
    IKAPI.node.create_child(&n3, n2,  IKAPI.to_ptr(3));
    IKAPI.node.create_child(&n4, n3,  IKAPI.to_ptr(4));
    IKAPI.node.create_child(&n5, n4,  IKAPI.to_ptr(5));
    IKAPI.node.create_child(&n6, n5,  IKAPI.to_ptr(6));
    IKAPI.node.create_child(&dead, tree,  IKAPI.to_ptr(7));
    IKAPI.node.create_child(&dead, n1,  IKAPI.to_ptr(8));
    IKAPI.node.create_child(&dead, n2,  IKAPI.to_ptr(9));
    IKAPI.node.create_child(&dead, n3,  IKAPI.to_ptr(10));
    IKAPI.node.create_child(&dead, n4,  IKAPI.to_ptr(11));
    IKAPI.node.create_child(&dead, n5,  IKAPI.to_ptr(12));
    IKAPI.node.create_child(&dead, n6,  IKAPI.to_ptr(13));

    ik_effector_t *e1, *e2, *e3;
    IKAPI.node.create_effector(&e1, n2);
    IKAPI.node.create_effector(&e2, n3);
    IKAPI.node.create_effector(&e3, n5);
    IKAPI.effector.set_chain_length(e3, 1);

    /*
     *   13--6
     *       |
     *   12--5 <- e3 (chain length = 1)
     *       |
     *   11--4
     *       |
     *   10--3 <- e2
     *       |
     *    9--2 <- e1
     *       |
     *    8--1
     *       |
     *    7--0
     *
     */

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(3));

    ik_ntf_t *ntf1, *ntf2, *ntf3;
    ntf1 = (ik_ntf_t*)vector_get_element(&ntf_list, 0);
    ntf2 = (ik_ntf_t*)vector_get_element(&ntf_list, 1);
    ntf3 = (ik_ntf_t*)vector_get_element(&ntf_list, 2);

    EXPECT_THAT(NTF_PRE_BASE(ntf1, 0), Eq(tree->node_data));
    EXPECT_THAT(ntf1->node_count, Eq(3));  // 0,1,2
    EXPECT_THAT(NTF_PRE_BASE(ntf2, 0), Eq(n2->node_data));
    EXPECT_THAT(ntf2->node_count, Eq(2));  // 2,3,4
    EXPECT_THAT(NTF_PRE_BASE(ntf3, 0), Eq(n4->node_data));
    EXPECT_THAT(ntf3->node_count, Eq(2));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}

TEST_F(NAME, check_ntf_list_order_for_disjoint_trees_llr)
{
    vector_t ntf_list;
    ik_node_t* tree = tree_llr();
    ik_node_t *e1, *e2, *e3;

    // Need to change effector chain lengths to tree becomes disjoint
    IKAPI.node.find_child(&e1, tree, IKAPI.to_ptr(6));
    IKAPI.node.find_child(&e2, tree, IKAPI.to_ptr(8));
    IKAPI.node.find_child(&e3, tree, IKAPI.to_ptr(10));
    IKAPI.effector.set_chain_length(IKAPI.node.get_effector(e1), 1);
    IKAPI.effector.set_chain_length(IKAPI.node.get_effector(e2), 1);

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(3));

    /*
     * The scenario here is nodes 0,1,2,9,10 form a chain (call this
     * "ntf 1"), nodes 5,6 form a chain ("ntf 2"), and nodes 7,8 form
     * a chain ("ntf 3"). Because ntf's 2 and 3 depend on the solution of
     * ntf 1, ntf 1 must appear before ntf's 2 and 3 in the ntf_list.
     *
     * e1 -> 6       8 <- e2
     *        \     /
     *         5   7
     *          \ /
     *    b2 ->  4      10 <- e3
     *            \     /
     *             3   9
     *              \ /
     *               2  <- b1
     *               |
     *               1
     *               |
     *               0
     */
    struct ik_node_t *b1, *b2;
    IKAPI.node.find_child(&b1, tree, IKAPI.to_ptr(5));  // expected base of second ntf
    IKAPI.node.find_child(&b2, tree, IKAPI.to_ptr(7));  // expected base of third ntf
    struct ik_ntf_t* ntf1 = (ik_ntf_t*)vector_get_element(&ntf_list, 0);
    struct ik_ntf_t* ntf2 = (ik_ntf_t*)vector_get_element(&ntf_list, 1);
    struct ik_ntf_t* ntf3 = (ik_ntf_t*)vector_get_element(&ntf_list, 2);

    EXPECT_THAT(NTF_PRE_BASE(ntf1, 0), Eq(tree->node_data));
    EXPECT_THAT(NTF_PRE_BASE(ntf2, 0), Eq(b1->node_data));
    EXPECT_THAT(NTF_PRE_BASE(ntf3, 0), Eq(b2->node_data));

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}

static void
ntf_print(ik_ntf_t* ntf)
{
    printf("Pre:");
    for (uint32_t i = 0; i != ntf->node_count; ++i)
        printf(" %d", (int)(intptr_t)(NTF_PRE_NODE(ntf, i)->user_data));
    printf("\n");
    printf("Post:");
    for (uint32_t i = 0; i != ntf->node_count; ++i)
        printf(" %d", (int)(intptr_t)(NTF_POST_NODE(ntf, i)->user_data));
    printf("\n");
}

TEST_F(NAME, check_ntf_list_order_for_disjoint_trees_llrr)
{
    vector_t ntf_list;
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9, *n10, *n11, *n12, *n13, *n14;

    IKAPI.node.create(&tree, IKAPI.to_ptr(0));
    IKAPI.node.create_child(&n1, tree, IKAPI.to_ptr(1));
    IKAPI.node.create_child(&n2,  n1,  IKAPI.to_ptr(2));
    IKAPI.node.create_child(&n3,  n2,  IKAPI.to_ptr(3));
    IKAPI.node.create_child(&n4,  n3,  IKAPI.to_ptr(4));
    IKAPI.node.create_child(&n5,  n4,  IKAPI.to_ptr(5));
    IKAPI.node.create_child(&n6,  n5,  IKAPI.to_ptr(6));
    IKAPI.node.create_child(&n7,  n4,  IKAPI.to_ptr(7));
    IKAPI.node.create_child(&n8,  n7,  IKAPI.to_ptr(8));
    IKAPI.node.create_child(&n9,  n2,  IKAPI.to_ptr(9));
    IKAPI.node.create_child(&n10, n9,  IKAPI.to_ptr(10));
    IKAPI.node.create_child(&n11, n10, IKAPI.to_ptr(11));
    IKAPI.node.create_child(&n12, n11, IKAPI.to_ptr(12));
    IKAPI.node.create_child(&n13, n10, IKAPI.to_ptr(13));
    IKAPI.node.create_child(&n14, n13, IKAPI.to_ptr(14));

    ik_effector_t *e1, *e2, *e3, *e4, *e5;
    IKAPI.node.create_effector(&e1, n6);
    IKAPI.node.create_effector(&e2, n8);
    IKAPI.node.create_effector(&e3, n12);
    IKAPI.node.create_effector(&e4, n14);
    IKAPI.node.create_effector(&e5, n2);
    IKAPI.effector.set_chain_length(e1, 3);
    IKAPI.effector.set_chain_length(e2, 1);
    IKAPI.effector.set_chain_length(e3, 1);
    IKAPI.effector.set_chain_length(e4, 4);

    /*
     * e1 : 3,4,5,6
     * e2 : 7,8
     * e3 : 11,12
     * e4 : 2,9,10,13,14
     * e5 : 0,1,2
     *
     *   e2 -> 8    e3 -> 12   14 <- e4
     *          \          |  /
     *           7        11 13
     *            \        |/
     * e1 -> 6--5--4       10
     *              \     /
     *               3   9
     *                \ /
     *                 2 <- e5
     *                 |
     *                 1
     *                 |
     *                 0
     */

    ik_ntf_list_construct(&ntf_list);
    ASSERT_THAT(ik_ntf_list_fill(&ntf_list, tree), IK_OK);
    ASSERT_THAT(vector_count(&ntf_list), Eq(5));
    struct ik_ntf_t* ntf1 = (ik_ntf_t*)vector_get_element(&ntf_list, 0);
    struct ik_ntf_t* ntf2 = (ik_ntf_t*)vector_get_element(&ntf_list, 1);
    struct ik_ntf_t* ntf3 = (ik_ntf_t*)vector_get_element(&ntf_list, 2);
    struct ik_ntf_t* ntf4 = (ik_ntf_t*)vector_get_element(&ntf_list, 3);
    struct ik_ntf_t* ntf5 = (ik_ntf_t*)vector_get_element(&ntf_list, 4);

    // Compare base nodes in NTF to check for the correct order
    // We expect base node order 0, 2, 3, 7, 11
    EXPECT_THAT(NTF_PRE_NODE(ntf1, 0), Eq(tree->node_data));
    EXPECT_THAT(NTF_PRE_NODE(ntf2, 0), Eq(n2->node_data));
    EXPECT_THAT(NTF_PRE_NODE(ntf3, 0), Eq(n3->node_data));
    EXPECT_THAT(NTF_PRE_NODE(ntf4, 0), Eq(n7->node_data));
    EXPECT_THAT(NTF_PRE_NODE(ntf5, 0), Eq(n11->node_data));

    // Quick sanity check to make sure we really are talking about the chains
    // we specified
    EXPECT_THAT(ntf1->node_count, Eq(3));  // 0,1,2
    EXPECT_THAT(ntf2->node_count, Eq(5));  // 2,9,10,13,14
    EXPECT_THAT(ntf3->node_count, Eq(4));  // 3,4,5,6
    EXPECT_THAT(ntf4->node_count, Eq(2));  // 7,8
    EXPECT_THAT(ntf5->node_count, Eq(2));  // 11,12

    ntf_print(ntf1);
    ntf_print(ntf2);
    ntf_print(ntf3);
    ntf_print(ntf4);
    ntf_print(ntf5);

    IKAPI.node.destroy_recursive(tree);
    ik_ntf_list_clear(&ntf_list);
}
