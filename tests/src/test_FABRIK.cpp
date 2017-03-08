#include "gmock/gmock.h"
#include "ik/solver.h"
#include "ik/node.h"
#include "ik/effector.h"

#define NAME FABRIK

TEST(NAME, weird_tree)
{
    solver_t* solver = ik_solver_create(ALGORITHM_FABRIK);
    node_t* root = node_create(0);
    node_t* child1 = node_create(1);
    node_t* child2 = node_create(2);
    node_t* child3 = node_create(3);
    node_t* child4 = node_create(4);
    node_t* child5 = node_create(5);
    node_t* child6 = node_create(6);
    node_t* child7 = node_create(7);
    node_t* child8 = node_create(8);
    node_t* child9 = node_create(9);
    node_t* child10 = node_create(10);
    node_t* child11 = node_create(11);
    node_t* child12 = node_create(12);
    node_add_child(root, child1);
    node_add_child(child1, child2);
    node_add_child(child2, child3);
    node_add_child(child3, child4);
    node_add_child(child2, child5);
    node_add_child(child5, child6);
    node_add_child(child6, child7);
    node_add_child(child6, child8);
    node_add_child(child8, child9);
    node_add_child(child9, child10);
    node_add_child(child10, child11);
    node_add_child(child11, child12);

    effector_t* eff1 = effector_create();
    effector_t* eff2 = effector_create();
    effector_t* eff3 = effector_create();
    node_attach_effector(child4, eff1);
    node_attach_effector(child7, eff2);
    node_attach_effector(child11, eff3);
    eff1->chain_length = 1;
    eff2->chain_length = 4;
    eff3->chain_length = 7;

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);

    ik_solver_destroy(solver);
}

TEST(NAME, just_one_node)
{
    solver_t* solver = ik_solver_create(ALGORITHM_FABRIK);
    node_t* root = node_create(0);
    effector_t* eff = effector_create();
    node_attach_effector(root, eff);
    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);
    ik_solver_destroy(solver);
}

TEST(NAME, two_arms_meet_at_same_node)
{
    solver_t* solver = ik_solver_create(ALGORITHM_FABRIK);
    node_t* root = node_create(0);
    node_t* child1 = node_create(1);
    node_t* child2 = node_create(2);
    node_t* child3 = node_create(3);
    node_t* child4 = node_create(4);
    node_t* child5 = node_create(5);
    node_t* child6 = node_create(6);
    node_add_child(root, child1);
    node_add_child(child1, child2);
    node_add_child(child2, child3);
    node_add_child(child3, child4);
    node_add_child(child2, child5);
    node_add_child(child5, child6);

    effector_t* eff1 = effector_create();
    effector_t* eff2 = effector_create();
    node_attach_effector(child4, eff1);
    node_attach_effector(child6, eff2);
    eff1->chain_length = 2;
    eff2->chain_length = 2;

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);
    ik_solver_destroy(solver);
}

TEST(NAME, effector_in_middle_of_chain)
{
    solver_t* solver = ik_solver_create(ALGORITHM_FABRIK);
    node_t* root = node_create(0);
    node_t* child1 = node_create(1);
    node_t* child2 = node_create(2);
    node_t* child3 = node_create(3);
    node_t* child4 = node_create(4);
    node_t* child5 = node_create(5);
    node_t* child6 = node_create(6);
    node_add_child(root, child1);
    node_add_child(child1, child2);
    node_add_child(child2, child3);
    node_add_child(child3, child4);
    node_add_child(child4, child5);
    node_add_child(child5, child6);

    effector_t* eff1 = effector_create();
    effector_t* eff2 = effector_create();
    node_attach_effector(child3, eff1);
    node_attach_effector(child6, eff2);

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);
    ik_solver_destroy(solver);
}

static void buildTree(node_t* parent, int depth, int* guid)
{
    node_t* child1 = node_create(++(*guid));
    node_t* child2 = node_create(++(*guid));
    node_t* child3 = node_create(++(*guid));
    node_t* child4 = node_create(++(*guid));
    node_t* child5 = node_create(++(*guid));
    node_t* child6 = node_create(++(*guid));
    node_add_child(parent, child1);
    node_add_child(child1, child2);
    node_add_child(child2, child3);

    node_add_child(parent, child4);
    node_add_child(child4, child5);
    node_add_child(child5, child6);

    if(depth < 5)
    {
        buildTree(child3, depth+1, guid);
        buildTree(child6, depth+1, guid);
    }
    else
    {
        effector_t* eff1 = effector_create();
        effector_t* eff2 = effector_create();
        node_attach_effector(child3, eff1);
        node_attach_effector(child6, eff2);
    }
}

TEST(NAME, binary_tree)
{
    int guid = 0;
    solver_t* solver = ik_solver_create(ALGORITHM_FABRIK);
    node_t* root = node_create(0);
    buildTree(root, 0, &guid);

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);
    ik_solver_destroy(solver);
}
