#include "ik/solver.h"
#include "ik/node.h"
#include "ik/effector.h"
#include "ik/memory.h"
#include "ik/log.h"

int main()
{
    ik_memory_init();
    ik_log_init();

    solver_t* solver = ik_solver_create(ALGORITHM_FABRIK);
    node_t* root = node_create(0);
    node_t* child1 = node_create(1);
    node_t* child2 = node_create(2);
    node_t* child3 = node_create(3);
    node_t* child4 = node_create(4);
    node_t* child5 = node_create(5);
    node_t* child6 = node_create(6);
    node_t* child7 = node_create(7);
    node_add_child(root, child1);
    node_add_child(child1, child2);
    node_add_child(child2, child3);
    node_add_child(child3, child4);
    node_add_child(child2, child5);
    node_add_child(child5, child6);
    node_add_child(child6, child7);

    child4->effector = effector_create();
    child7->effector = effector_create();

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);

    ik_solver_destroy(solver);

    ik_log_deinit();
    ik_memory_deinit();
    return 0;
}
