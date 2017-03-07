#include "ik/solver.h"
#include "ik/node.h"
#include "ik/effector.h"
#include "ik/memory.h"
#include "ik/log.h"

void scenario1()
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
    eff1->chain_length = 3;
    eff2->chain_length = 4;
    eff3->chain_length = 7;

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);

    node_dump_to_dot(root, "tree.dot");

    ik_solver_destroy(solver);
}

void scenario2()
{
    solver_t* solver = ik_solver_create(ALGORITHM_FABRIK);
    node_t* root = node_create(0);
    effector_t* eff = effector_create();
    node_attach_effector(root, eff);
    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);
    ik_solver_destroy(solver);
}

void scenario3()
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
    eff1->chain_length = 1;

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);
    ik_solver_destroy(solver);
}

int main()
{
    ik_memory_init();
    ik_log_init(LOG_STDOUT);

    scenario1();
    scenario2();
    scenario3();

    ik_log_deinit();
    ik_memory_deinit();
    return 0;
}
