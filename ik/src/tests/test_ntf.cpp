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

    IKAPI.node.destroy_recursive(tree);
    EXPECT_THAT(ntf->node_data->refcount->refs, Eq(1));  /* ntf structure should still hold a reference */
    ik_ntf_list_destroy(ntf_list);
}
