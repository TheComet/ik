#include "gmock/gmock.h"
#include "ik/solver.h"
#include "ik/node.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/solver_FABRIK.h"

#define NAME FABRIK

using namespace ::testing;

class NAME : public Test
{
public:
    NAME() : solver(NULL) {}

    virtual void SetUp()
    {
        solver = ik_solver_create(SOLVER_FABRIK);
    }

    virtual void TearDown()
    {
        ik_solver_destroy(solver);
    }

protected:
    ik_solver_t* solver;
};

TEST_F(NAME, weird_tree)
{
    ik_node_t* root = ik_node_create(0);
    ik_node_t* child1 = ik_node_create(1);
    ik_node_t* child2 = ik_node_create(2);
    ik_node_t* child3 = ik_node_create(3);
    ik_node_t* child4 = ik_node_create(4);
    ik_node_t* child5 = ik_node_create(5);
    ik_node_t* child6 = ik_node_create(6);
    ik_node_t* child7 = ik_node_create(7);
    ik_node_t* child8 = ik_node_create(8);
    ik_node_t* child9 = ik_node_create(9);
    ik_node_t* child10 = ik_node_create(10);
    ik_node_t* child11 = ik_node_create(11);
    ik_node_t* child12 = ik_node_create(12);
    ik_node_add_child(root, child1);
    ik_node_add_child(child1, child2);
    ik_node_add_child(child2, child3);
    ik_node_add_child(child3, child4);
    ik_node_add_child(child2, child5);
    ik_node_add_child(child5, child6);
    ik_node_add_child(child6, child7);
    ik_node_add_child(child6, child8);
    ik_node_add_child(child8, child9);
    ik_node_add_child(child9, child10);
    ik_node_add_child(child10, child11);
    ik_node_add_child(child11, child12);

    ik_effector_t* eff1 = ik_effector_create();
    ik_effector_t* eff2 = ik_effector_create();
    ik_effector_t* eff3 = ik_effector_create();
    ik_node_attach_effector(child4, eff1);
    ik_node_attach_effector(child7, eff2);
    ik_node_attach_effector(child11, eff3);
    eff1->chain_length = 1;
    eff2->chain_length = 4;
    eff3->chain_length = 7;

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_chain_trees(solver);
    ik_solver_solve(solver);

    fabrik_t* fabrik = (fabrik_t*)solver;

    // There are two separate chain trees
    ASSERT_THAT(ordered_vector_count(&fabrik->chain_tree.islands), Eq(2u));

    // First has length 2
    chain_t* chain1 = (chain_t*)ordered_vector_get_element(&fabrik->chain_tree.islands, 0);
    ASSERT_THAT(ordered_vector_count(&chain1->nodes), Eq(2u));
    ik_node_t* node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 0);
    EXPECT_THAT(node->guid, Eq(4u));
    EXPECT_THAT(node, Eq(child4));
    EXPECT_THAT(node->effector, NotNull());
    node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 1);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));

    // Second has length 4
    chain_t* chain2 = (chain_t*)ordered_vector_get_element(&fabrik->chain_tree.islands, 1);
    ASSERT_THAT(ordered_vector_count(&chain2->nodes), Eq(4u));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 0);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 1);
    EXPECT_THAT(node->guid, Eq(5u));
    EXPECT_THAT(node, Eq(child5));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 2);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 3);
    EXPECT_THAT(node->guid, Eq(1u));
    EXPECT_THAT(node, Eq(child1));

    // First Sub-chain with length 2
    ASSERT_THAT(ordered_vector_count(&chain2->children), Eq(2u));
    chain_t* chain3 = (chain_t*)ordered_vector_get_element(&chain2->children, 0);
    ASSERT_THAT(ordered_vector_count(&chain3->nodes), Eq(2u));
    node = *(ik_node_t**)ordered_vector_get_element(&chain3->nodes, 0);
    EXPECT_THAT(node->guid, Eq(7u));
    EXPECT_THAT(node, Eq(child7));
    EXPECT_THAT(node->effector, NotNull());
    node = *(ik_node_t**)ordered_vector_get_element(&chain3->nodes, 1);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));

    // Second sub-chain with length 5
    chain_t* chain4 = (chain_t*)ordered_vector_get_element(&chain2->children, 1);
    ASSERT_THAT(ordered_vector_count(&chain4->nodes), Eq(5u));
    node = *(ik_node_t**)ordered_vector_get_element(&chain4->nodes, 0);
    EXPECT_THAT(node->guid, Eq(11u));
    EXPECT_THAT(node, Eq(child11));
    EXPECT_THAT(node->effector, NotNull());
    node = *(ik_node_t**)ordered_vector_get_element(&chain4->nodes, 1);
    EXPECT_THAT(node->guid, Eq(10u));
    EXPECT_THAT(node, Eq(child10));
    node = *(ik_node_t**)ordered_vector_get_element(&chain4->nodes, 2);
    EXPECT_THAT(node->guid, Eq(9u));
    EXPECT_THAT(node, Eq(child9));
    node = *(ik_node_t**)ordered_vector_get_element(&chain4->nodes, 3);
    EXPECT_THAT(node->guid, Eq(8u));
    EXPECT_THAT(node, Eq(child8));
    node = *(ik_node_t**)ordered_vector_get_element(&chain4->nodes, 4);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));

    // These chains should have no children
    EXPECT_THAT(ordered_vector_count(&chain1->children), Eq(0u));
    EXPECT_THAT(ordered_vector_count(&chain3->children), Eq(0u));
    EXPECT_THAT(ordered_vector_count(&chain4->children), Eq(0u));
}

TEST_F(NAME, just_one_node)
{
    ik_node_t* root = ik_node_create(0);
    ik_effector_t* eff = ik_effector_create();
    ik_node_attach_effector(root, eff);
    ik_solver_set_tree(solver, root);
	ik_solver_rebuild_chain_trees(solver);
    ik_solver_solve(solver);

    // We expect no chains to be created
    fabrik_t* fabrik = (fabrik_t*)solver;
    ASSERT_THAT(ordered_vector_count(&fabrik->chain_tree.islands), Eq(0u));
}

TEST_F(NAME, two_arms_meet_at_same_node)
{
    ik_node_t* root = ik_node_create(0);
    ik_node_t* child1 = ik_node_create(1);
    ik_node_t* child2 = ik_node_create(2);
    ik_node_t* child3 = ik_node_create(3);
    ik_node_t* child4 = ik_node_create(4);
    ik_node_t* child5 = ik_node_create(5);
    ik_node_t* child6 = ik_node_create(6);
    ik_node_add_child(root, child1);
    ik_node_add_child(child1, child2);
    ik_node_add_child(child2, child3);
    ik_node_add_child(child3, child4);
    ik_node_add_child(child2, child5);
    ik_node_add_child(child5, child6);

    ik_effector_t* eff1 = ik_effector_create();
    ik_effector_t* eff2 = ik_effector_create();
    ik_node_attach_effector(child4, eff1);
    ik_node_attach_effector(child6, eff2);
    eff1->chain_length = 2;
    eff2->chain_length = 2;

    ik_solver_set_tree(solver, root);
	ik_solver_rebuild_chain_trees(solver);
    ik_solver_solve(solver);

    fabrik_t* fabrik = (fabrik_t*)solver;
    ASSERT_THAT(ordered_vector_count(&fabrik->chain_tree.islands), Eq(2u));
    chain_t* chain1 = (chain_t*)ordered_vector_get_element(&fabrik->chain_tree.islands, 0);
    chain_t* chain2 = (chain_t*)ordered_vector_get_element(&fabrik->chain_tree.islands, 1);

    // First arm
    ASSERT_THAT(ordered_vector_count(&chain1->nodes), Eq(3u));
    ik_node_t* node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 0);
    EXPECT_THAT(node->guid, Eq(4u));
    EXPECT_THAT(node, Eq(child4));
    EXPECT_THAT(node->effector, NotNull());
    node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 1);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));
    node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 2);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));

    // Second arm
    ASSERT_THAT(ordered_vector_count(&chain2->nodes), Eq(3u));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 0);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));
    EXPECT_THAT(node->effector, NotNull());
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 1);
    EXPECT_THAT(node->guid, Eq(5u));
    EXPECT_THAT(node, Eq(child5));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 2);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));

    // These chains should have no children
    EXPECT_THAT(ordered_vector_count(&chain1->children), Eq(0u));
    EXPECT_THAT(ordered_vector_count(&chain2->children), Eq(0u));
}

TEST_F(NAME, two_separate_arms)
{
    ik_node_t* root = ik_node_create(0);
    ik_node_t* child1 = ik_node_create(1);
    ik_node_t* child2 = ik_node_create(2);
    ik_node_t* child3 = ik_node_create(3);
    ik_node_t* child4 = ik_node_create(4);
    ik_node_t* child5 = ik_node_create(5);
    ik_node_add_child(root, child1);
    ik_node_add_child(child1, child2);
    ik_node_add_child(child2, child3);
    ik_node_add_child(child1, child4);
    ik_node_add_child(child4, child5);

    ik_effector_t* eff1 = ik_effector_create();
    ik_effector_t* eff2 = ik_effector_create();
    ik_node_attach_effector(child3, eff1);
    ik_node_attach_effector(child5, eff2);
    eff1->chain_length = 1;
    eff2->chain_length = 1;

    ik_solver_set_tree(solver, root);
	ik_solver_rebuild_chain_trees(solver);
    ik_solver_solve(solver);

    fabrik_t* fabrik = (fabrik_t*)solver;
    ASSERT_THAT(ordered_vector_count(&fabrik->chain_tree.islands), Eq(2u));
    chain_t* chain1 = (chain_t*)ordered_vector_get_element(&fabrik->chain_tree.islands, 0);
    chain_t* chain2 = (chain_t*)ordered_vector_get_element(&fabrik->chain_tree.islands, 1);

    // First arm
    ASSERT_THAT(ordered_vector_count(&chain1->nodes), Eq(2u));
    ik_node_t* node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 0);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));
    EXPECT_THAT(node->effector, NotNull());
    node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 1);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));

    // Second arm
    ASSERT_THAT(ordered_vector_count(&chain2->nodes), Eq(2u));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 0);
    EXPECT_THAT(node->guid, Eq(5u));
    EXPECT_THAT(node, Eq(child5));
    EXPECT_THAT(node->effector, NotNull());
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 1);
    EXPECT_THAT(node->guid, Eq(4u));
    EXPECT_THAT(node, Eq(child4));

    // These chains should have no children
    EXPECT_THAT(ordered_vector_count(&chain1->children), Eq(0u));
    EXPECT_THAT(ordered_vector_count(&chain2->children), Eq(0u));
}

TEST_F(NAME, effector_in_middle_of_chain)
{
    ik_node_t* root = ik_node_create(0);
    ik_node_t* child1 = ik_node_create(1);
    ik_node_t* child2 = ik_node_create(2);
    ik_node_t* child3 = ik_node_create(3);
    ik_node_t* child4 = ik_node_create(4);
    ik_node_t* child5 = ik_node_create(5);
    ik_node_t* child6 = ik_node_create(6);
    ik_node_add_child(root, child1);
    ik_node_add_child(child1, child2);
    ik_node_add_child(child2, child3);
    ik_node_add_child(child3, child4);
    ik_node_add_child(child4, child5);
    ik_node_add_child(child5, child6);

    ik_effector_t* eff1 = ik_effector_create();
    ik_effector_t* eff2 = ik_effector_create();
    ik_node_attach_effector(child3, eff1);
    ik_node_attach_effector(child6, eff2);

    ik_solver_set_tree(solver, root);
	ik_solver_rebuild_chain_trees(solver);
    ik_solver_solve(solver);

    // We expect the chain to be broken into 2 parts, one as a child of the other
    fabrik_t* fabrik = (fabrik_t*)solver;
    ASSERT_THAT(ordered_vector_count(&fabrik->chain_tree.islands), Eq(1u));
    chain_t* chain1 = (chain_t*)ordered_vector_get_element(&fabrik->chain_tree.islands, 0);
    ASSERT_THAT(ordered_vector_count(&chain1->children), Eq(1u));
    chain_t* chain2 = (chain_t*)ordered_vector_get_element(&chain1->children, 0);

    // Bottom section
    ASSERT_THAT(ordered_vector_count(&chain1->nodes), Eq(4u));
    ik_node_t* node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 0);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));
    EXPECT_THAT(node->effector, NotNull());
    node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 1);
    EXPECT_THAT(node->guid, Eq(2u));
    EXPECT_THAT(node, Eq(child2));
    node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 2);
    EXPECT_THAT(node->guid, Eq(1u));
    EXPECT_THAT(node, Eq(child1));
    node = *(ik_node_t**)ordered_vector_get_element(&chain1->nodes, 3);
    EXPECT_THAT(node->guid, Eq(0u));
    EXPECT_THAT(node, Eq(root));

    // Top section
    ASSERT_THAT(ordered_vector_count(&chain2->nodes), Eq(4u));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 0);
    EXPECT_THAT(node->guid, Eq(6u));
    EXPECT_THAT(node, Eq(child6));
    EXPECT_THAT(node->effector, NotNull());
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 1);
    EXPECT_THAT(node->guid, Eq(5u));
    EXPECT_THAT(node, Eq(child5));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 2);
    EXPECT_THAT(node->guid, Eq(4u));
    EXPECT_THAT(node, Eq(child4));
    node = *(ik_node_t**)ordered_vector_get_element(&chain2->nodes, 3);
    EXPECT_THAT(node->guid, Eq(3u));
    EXPECT_THAT(node, Eq(child3));

    // These chains should have no children
    EXPECT_THAT(ordered_vector_count(&chain2->children), Eq(0u));
}

static void buildTreeLongChains(ik_node_t* parent, int depth, int* guid)
{
    ik_node_t* child1 = ik_node_create(++(*guid));
    ik_node_t* child2 = ik_node_create(++(*guid));
    ik_node_t* child3 = ik_node_create(++(*guid));
    ik_node_t* child4 = ik_node_create(++(*guid));
    ik_node_t* child5 = ik_node_create(++(*guid));
    ik_node_t* child6 = ik_node_create(++(*guid));
    ik_node_add_child(parent, child1);
    ik_node_add_child(child1, child2);
    ik_node_add_child(child2, child3);

    ik_node_add_child(parent, child4);
    ik_node_add_child(child4, child5);
    ik_node_add_child(child5, child6);

    if(depth < 4)
    {
        buildTreeLongChains(child3, depth+1, guid);
        buildTreeLongChains(child6, depth+1, guid);
    }
    else
    {
        ik_effector_t* eff1 = ik_effector_create();
        ik_effector_t* eff2 = ik_effector_create();
        ik_node_attach_effector(child3, eff1);
        ik_node_attach_effector(child6, eff2);
    }
}

TEST_F(NAME, binary_tree_with_long_chains)
{
    int guid = 0;
    ik_node_t* root = ik_node_create(0);
    buildTreeLongChains(root, 0, &guid);

    ik_solver_set_tree(solver, root);
	ik_solver_rebuild_chain_trees(solver);
    ik_solver_solve(solver);
}

static void buildTreeShortChains(ik_node_t* parent, int depth, int* guid)
{
    ik_node_t* child1 = ik_node_create(++(*guid));
    ik_node_t* child2 = ik_node_create(++(*guid));
    ik_node_add_child(parent, child1);
    ik_node_add_child(parent, child2);

    if(depth < 4)
    {
        buildTreeLongChains(child1, depth+1, guid);
        buildTreeLongChains(child2, depth+1, guid);
    }
    else
    {
        ik_effector_t* eff1 = ik_effector_create();
        ik_effector_t* eff2 = ik_effector_create();
        ik_node_attach_effector(child1, eff1);
        ik_node_attach_effector(child2, eff2);
    }
}

TEST_F(NAME, binary_tree_with_short_chains)
{
    int guid = 0;
    ik_node_t* root = ik_node_create(0);
    buildTreeShortChains(root, 0, &guid);

    ik_solver_set_tree(solver, root);
	ik_solver_rebuild_chain_trees(solver);
    ik_solver_solve(solver);
}
