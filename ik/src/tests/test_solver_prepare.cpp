#include <gmock/gmock.h>
#include "ik/ik.h"
#include "ik/algorithm_prepare.h"
#include "ik/node.h"

#define NAME algorithm_prepare

using namespace testing;

struct NAME : public Test
{
    void SetUp() override
    {
        IKAPI.algorithm.create(&algorithm, IKAPI.algorithm.ONE_BONE);
    }

    void TearDown() override
    {
        IKAPI.algorithm.destroy(algorithm);
    }

    ik_node_t* tree_with_no_effectors()
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

    ik_node_t* tree_with_one_effector()
    {
        ik_node_t* n6;
        ik_node_t* tree = tree_with_no_effectors();
        IKAPI.node.find_child(&n6, tree, IKAPI.to_ptr(6));

        ik_effector_t *eff1;
        IKAPI.node.create_effector(&eff1, n6);

        return tree;
    }

    ik_node_t* tree_with_two_effectors()
    {
        ik_node_t *n6, *n9;
        ik_node_t* tree = tree_with_no_effectors();
        IKAPI.node.find_child(&n6, tree, IKAPI.to_ptr(6));
        IKAPI.node.find_child(&n9, tree, IKAPI.to_ptr(9));

        ik_effector_t *eff1, *eff2;
        IKAPI.node.create_effector(&eff1, n6);
        IKAPI.node.create_effector(&eff2, n9);

        return tree;
    }

    ik_algorithm_t* algorithm;
};
