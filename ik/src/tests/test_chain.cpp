#include "gmock/gmock.h"
#include "ik/ik.h"
#include "ik/chain.h"
#include "ik/solverdef.h"

/* Need access to solver->chain_list */
struct ik_solver_t
{
    SOLVER_HEAD
};

#define NAME chain

using namespace ::testing;

class NAME : public Test
{
public:
    NAME() : solver(NULL) {}

    virtual void SetUp()
    {
        IKAPI.solver.create(&solver, IKAPI.solver.FABRIK);
    }

    virtual void TearDown()
    {
        IKAPI.solver.destroy(solver);
    }

protected:
    ik_solver_t* solver;
};

TEST_F(NAME, weird_tree)
{
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* child1 = IKAPI.node.create(1);
    ik_node_t* child2 = IKAPI.node.create(2);
    ik_node_t* child3 = IKAPI.node.create(3);
    ik_node_t* child4 = IKAPI.node.create(4);
    ik_node_t* child5 = IKAPI.node.create(5);
    ik_node_t* child6 = IKAPI.node.create(6);
    ik_node_t* child7 = IKAPI.node.create(7);
    ik_node_t* child8 = IKAPI.node.create(8);
    ik_node_t* child9 = IKAPI.node.create(9);
    ik_node_t* child10 = IKAPI.node.create(10);
    ik_node_t* child11 = IKAPI.node.create(11);
    ik_node_t* child12 = IKAPI.node.create(12);
    IKAPI.node.add_child(root, child1);
    IKAPI.node.add_child(child1, child2);
    IKAPI.node.add_child(child2, child3);
    IKAPI.node.add_child(child3, child4);
    IKAPI.node.add_child(child2, child5);
    IKAPI.node.add_child(child5, child6);
    IKAPI.node.add_child(child6, child7);
    IKAPI.node.add_child(child6, child8);
    IKAPI.node.add_child(child8, child9);
    IKAPI.node.add_child(child9, child10);
    IKAPI.node.add_child(child10, child11);
    IKAPI.node.add_child(child11, child12);

    ik_effector_t* eff1 = IKAPI.effector.create();
    ik_effector_t* eff2 = IKAPI.effector.create();
    ik_effector_t* eff3 = IKAPI.effector.create();
    IKAPI.effector.attach(eff1, child4);
    IKAPI.effector.attach(eff2, child7);
    IKAPI.effector.attach(eff3, child11);
    eff1->chain_length = 1;
    eff2->chain_length = 4;
    eff3->chain_length = 7;

    IKAPI.solver.set_tree(solver, root);
    IKAPI.solver.rebuild(solver);

    // There are two separate chain trees
    ASSERT_THAT(vector_count(&solver->chain_list), Eq(2u));

    // First has length 2
    ik_chain* chain1 = (chain_t*)vector_get_element(&solver->chain_list, 0);
    ASSERT_THAT(chain_length(chain1), Eq(2u));
    ik_node_t* node = chain_get_node(chain1, 0);
    EXPECT_THAT(node->guid, Eq(4u));
    EXPECT_THAT(node, Eq(child4));
    EXPECT_THAT(node->effector, NotNull());
    node = chain_get_node(chain1, 1);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));

    // Second has length 4
    ik_chain* chain2 = (chain_t*)vector_get_element(&solver->chain_list, 1);
    ASSERT_THAT(chain_length(chain2), Eq(4u));
    node = chain_get_node(chain2, 0);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));
    node = chain_get_node(chain2, 1);
    EXPECT_THAT(node->guid, Eq(5u));
    EXPECT_THAT(node, Eq(child5));
    node = chain_get_node(chain2, 2);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));
    node = chain_get_node(chain2, 3);
    EXPECT_THAT(node->guid, Eq(1u));
    EXPECT_THAT(node, Eq(child1));

    // First Sub-chain with length 2
    ASSERT_THAT(chain_child_count(chain2), Eq(2u));
    ik_chain* chain3 = chain_get_child(chain2, 0);
    ASSERT_THAT(chain_length(chain3), Eq(2u));
    node = chain_get_node(chain3, 0);
    EXPECT_THAT(node->guid, Eq(7u));
    EXPECT_THAT(node, Eq(child7));
    EXPECT_THAT(node->effector, NotNull());
    node = chain_get_node(chain3, 1);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));

    // Second sub-chain with length 5
    ik_chain* chain4 = chain_get_child(chain2, 1);
    ASSERT_THAT(chain_length(chain4), Eq(5u));
    node = chain_get_node(chain4, 0);
    EXPECT_THAT(node->guid, Eq(11u));
    EXPECT_THAT(node, Eq(child11));
    EXPECT_THAT(node->effector, NotNull());
    node = chain_get_node(chain4, 1);
    EXPECT_THAT(node->guid, Eq(10u));
    EXPECT_THAT(node, Eq(child10));
    node = chain_get_node(chain4, 2);
    EXPECT_THAT(node->guid, Eq(9u));
    EXPECT_THAT(node, Eq(child9));
    node = chain_get_node(chain4, 3);
    EXPECT_THAT(node->guid, Eq(8u));
    EXPECT_THAT(node, Eq(child8));
    node = chain_get_node(chain4, 4);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));

    // These chains should have no children
    EXPECT_THAT(chain_child_count(chain1), Eq(0u));
    EXPECT_THAT(chain_child_count(chain3), Eq(0u));
    EXPECT_THAT(chain_child_count(chain4), Eq(0u));
}

TEST_F(NAME, just_one_node)
{
    ik_node_t* root = IKAPI.node.create(0);
    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.effector.attach(eff, root);
    IKAPI.solver.set_tree(solver, root);
    IKAPI.solver.rebuild(solver);

    // We expect no chains to be created
    ASSERT_THAT(vector_count(&solver->chain_list), Eq(0u));
}

TEST_F(NAME, two_arms_meet_at_same_node)
{
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* child1 = IKAPI.node.create(1);
    ik_node_t* child2 = IKAPI.node.create(2);
    ik_node_t* child3 = IKAPI.node.create(3);
    ik_node_t* child4 = IKAPI.node.create(4);
    ik_node_t* child5 = IKAPI.node.create(5);
    ik_node_t* child6 = IKAPI.node.create(6);
    IKAPI.node.add_child(root, child1);
    IKAPI.node.add_child(child1, child2);
    IKAPI.node.add_child(child2, child3);
    IKAPI.node.add_child(child3, child4);
    IKAPI.node.add_child(child2, child5);
    IKAPI.node.add_child(child5, child6);

    ik_effector_t* eff1 = IKAPI.effector.create();
    ik_effector_t* eff2 = IKAPI.effector.create();
    IKAPI.effector.attach(eff1, child4);
    IKAPI.effector.attach(eff2, child6);
    eff1->chain_length = 2;
    eff2->chain_length = 2;

    IKAPI.solver.set_tree(solver, root);
    IKAPI.solver.rebuild(solver);

    ASSERT_THAT(vector_count(&solver->chain_list), Eq(2u));
    ik_chain* chain1 = (chain_t*)vector_get_element(&solver->chain_list, 0);
    ik_chain* chain2 = (chain_t*)vector_get_element(&solver->chain_list, 1);

    // First arm
    ASSERT_THAT(chain_length(chain1), Eq(3u));
    ik_node_t* node = chain_get_node(chain1, 0);
    EXPECT_THAT(node->guid, Eq(4u));
    EXPECT_THAT(node, Eq(child4));
    EXPECT_THAT(node->effector, NotNull());
    node = chain_get_node(chain1, 1);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));
    node = chain_get_node(chain1, 2);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));

    // Second arm
    ASSERT_THAT(chain_length(chain2), Eq(3u));
    node = chain_get_node(chain2, 0);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));
    EXPECT_THAT(node->effector, NotNull());
    node = chain_get_node(chain2, 1);
    EXPECT_THAT(node->guid, Eq(5u));
    EXPECT_THAT(node, Eq(child5));
    node = chain_get_node(chain2, 2);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));

    // These chains should have no children
    EXPECT_THAT(chain_child_count(chain1), Eq(0u));
    EXPECT_THAT(chain_child_count(chain2), Eq(0u));
}

TEST_F(NAME, two_separate_arms)
{
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* child1 = IKAPI.node.create(1);
    ik_node_t* child2 = IKAPI.node.create(2);
    ik_node_t* child3 = IKAPI.node.create(3);
    ik_node_t* child4 = IKAPI.node.create(4);
    ik_node_t* child5 = IKAPI.node.create(5);
    IKAPI.node.add_child(root, child1);
    IKAPI.node.add_child(child1, child2);
    IKAPI.node.add_child(child2, child3);
    IKAPI.node.add_child(child1, child4);
    IKAPI.node.add_child(child4, child5);

    ik_effector_t* eff1 = IKAPI.effector.create();
    ik_effector_t* eff2 = IKAPI.effector.create();
    IKAPI.effector.attach(eff1, child3);
    IKAPI.effector.attach(eff2, child5);
    eff1->chain_length = 1;
    eff2->chain_length = 1;

    IKAPI.solver.set_tree(solver, root);
    IKAPI.solver.rebuild(solver);

    ASSERT_THAT(vector_count(&solver->chain_list), Eq(2u));
    ik_chain* chain1 = (chain_t*)vector_get_element(&solver->chain_list, 0);
    ik_chain* chain2 = (chain_t*)vector_get_element(&solver->chain_list, 1);

    // First arm
    ASSERT_THAT(chain_length(chain1), Eq(2u));
    ik_node_t* node = chain_get_node(chain1, 0);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));
    EXPECT_THAT(node->effector, NotNull());
    node = chain_get_node(chain1, 1);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));

    // Second arm
    ASSERT_THAT(chain_length(chain2), Eq(2u));
    node = chain_get_node(chain2, 0);
    EXPECT_THAT(node->guid, Eq(5u));
    EXPECT_THAT(node, Eq(child5));
    EXPECT_THAT(node->effector, NotNull());
    node = chain_get_node(chain2, 1);
    EXPECT_THAT(node->guid, Eq(4u));
    EXPECT_THAT(node, Eq(child4));

    // These chains should have no children
    EXPECT_THAT(chain_child_count(chain1), Eq(0u));
    EXPECT_THAT(chain_child_count(chain2), Eq(0u));
}

TEST_F(NAME, effector_in_middle_of_chain)
{
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* child1 = IKAPI.node.create(1);
    ik_node_t* child2 = IKAPI.node.create(2);
    ik_node_t* child3 = IKAPI.node.create(3);
    ik_node_t* child4 = IKAPI.node.create(4);
    ik_node_t* child5 = IKAPI.node.create(5);
    ik_node_t* child6 = IKAPI.node.create(6);
    IKAPI.node.add_child(root, child1);
    IKAPI.node.add_child(child1, child2);
    IKAPI.node.add_child(child2, child3);
    IKAPI.node.add_child(child3, child4);
    IKAPI.node.add_child(child4, child5);
    IKAPI.node.add_child(child5, child6);

    ik_effector_t* eff1 = IKAPI.effector.create();
    ik_effector_t* eff2 = IKAPI.effector.create();
    IKAPI.effector.attach(eff1, child3);
    IKAPI.effector.attach(eff2, child6);

    IKAPI.solver.set_tree(solver, root);
    IKAPI.solver.rebuild(solver);

    // We expect the chain to be broken into 2 parts, one as a child of the other
    ASSERT_THAT(vector_count(&solver->chain_list), Eq(1u));
    ik_chain* chain1 = (chain_t*)vector_get_element(&solver->chain_list, 0);
    ASSERT_THAT(chain_child_count(chain1), Eq(1u));
    ik_chain* chain2 = chain_get_child(chain1, 0);

    // Bottom section
    ASSERT_THAT(chain_length(chain1), Eq(4u));
    ik_node_t* node = chain_get_node(chain1, 0);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));
    EXPECT_THAT(node->effector, NotNull());
    node = chain_get_node(chain1, 1);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));
    node = chain_get_node(chain1, 2);
    EXPECT_THAT(node->guid, Eq(1u));
    EXPECT_THAT(node, Eq(child1));
    node = chain_get_node(chain1, 3);
    EXPECT_THAT(node->guid, Eq(0u));
    EXPECT_THAT(node, Eq(root));

    // Top section
    ASSERT_THAT(chain_length(chain2), Eq(4u));
    node = chain_get_node(chain2, 0);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));
    EXPECT_THAT(node->effector, NotNull());
    node = chain_get_node(chain2, 1);
    EXPECT_THAT(node->guid, Eq(5u));
    EXPECT_THAT(node, Eq(child5));
    node = chain_get_node(chain2, 2);
    EXPECT_THAT(node->guid, Eq(4u));
    EXPECT_THAT(node, Eq(child4));
    node = chain_get_node(chain2, 3);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));

    // These chains should have no children
    EXPECT_THAT(chain_child_count(chain2), Eq(0u));
}
