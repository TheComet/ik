#include "benchmark/benchmark.h"
#include "ik/ik.h"

using namespace benchmark;

enum Type
{
    CHAIN_10,
    TWO_ARMS,
    BINARY_TREE
};

static void build_tree_long_chains(ik_solver_t* solver, ik_node_t* parent, int depth, int* guid)
{
    ik_node_t* child1 = solver->node->create((*guid)++); child1->position.x = (depth*7) + 1; child1->position.y = (depth*7) + 1;
    ik_node_t* child2 = solver->node->create((*guid)++); child1->position.x = (depth*7) + 2; child1->position.y = (depth*7) + 2;
    ik_node_t* child3 = solver->node->create((*guid)++); child1->position.x = (depth*7) + 3; child1->position.y = (depth*7) + 3;
    ik_node_t* child4 = solver->node->create((*guid)++); child1->position.x = (depth*7) + 4; child1->position.y = (depth*7) + 4;
    ik_node_t* child5 = solver->node->create((*guid)++); child1->position.x = (depth*7) + 5; child1->position.y = (depth*7) + 5;
    ik_node_t* child6 = solver->node->create((*guid)++); child1->position.x = (depth*7) + 6; child1->position.y = (depth*7) + 6;
    solver->node->add_child(parent, child1);
    solver->node->add_child(child1, child2);
    solver->node->add_child(child2, child3);

    solver->node->add_child(parent, child4);
    solver->node->add_child(child4, child5);
    solver->node->add_child(child5, child6);

    if(depth > 0)
    {
        build_tree_long_chains(solver, child3, depth-1, guid);
        build_tree_long_chains(solver, child6, depth-1, guid);
    }
    else
    {
        ik_effector_t* eff1 = solver->effector->create();
        ik_effector_t* eff2 = solver->effector->create();
        eff1->target_position.x = depth * 100;
        eff2->target_position.z = depth * 100;
        solver->effector->attach(eff1, child3);
        solver->effector->attach(eff2, child6);
    }
}

static ik_node_t* create_tree(ik_solver_t* solver, Type type)
{
    static int guid = 0;
    ik_node_t* root = solver->node->create(guid++);

    switch (type)
    {
        case CHAIN_10:
        {
            ik_node_t* parent = root;
            for (int i = 0; i != 10; ++i)
            {
                ik_node_t* child = solver->node->create(guid++);
                child->position.y = i + 1; // 1 unit higher every time
                solver->node->add_child(parent, child);
                parent = child;
            }
            ik_effector_t* eff = solver->effector->create();
            eff->target_position.x = 5;
            eff->target_position.y = 0;
            solver->effector->attach(eff, parent);
        } break;

        case TWO_ARMS:
        {
            ik_node_t* child1 = solver->node->create(guid++); child1->position.y = 1;
            ik_node_t* child2 = solver->node->create(guid++); child2->position.y = 2;
            ik_node_t* child3 = solver->node->create(guid++); child3->position.y = 3;
            solver->node->add_child(root, child1);
            solver->node->add_child(child1, child2);
            solver->node->add_child(child2, child3);

            ik_node_t* sub_base = child3;

            child1 = solver->node->create(guid++); child1->position.y = 4; child1->position.x = -1;
            child2 = solver->node->create(guid++); child2->position.y = 5; child2->position.x = -2;
            child3 = solver->node->create(guid++); child2->position.y = 6; child2->position.x = -3;
            solver->node->add_child(sub_base, child1);
            solver->node->add_child(child1, child2);
            solver->node->add_child(child2, child3);

            ik_effector_t* eff = solver->effector->create();
            eff->target_position.z = 2; // make it grab forwards
            solver->effector->attach(eff, child3);

            child1 = solver->node->create(guid++); child1->position.y = 4; child1->position.x = 1;
            child2 = solver->node->create(guid++); child2->position.y = 5; child2->position.x = 2;
            child3 = solver->node->create(guid++); child2->position.y = 6; child2->position.x = 3;
            solver->node->add_child(sub_base, child1);
            solver->node->add_child(child1, child2);
            solver->node->add_child(child2, child3);

            eff = solver->effector->create();
            eff->target_position.z = 2; // make it grab forwards
            solver->effector->attach(eff, child3);
        } break;

        case BINARY_TREE:
        {
            build_tree_long_chains(solver, root, 10, &guid);
        } break;
    };

    return root;
}

static ik_solver_t* create_solver(Type type)
{
    ik_solver_t* solver = IKAPI.solver.create(IK_FABRIK);
    ik_node_t* root = create_tree(solver, type);
    IKAPI.solver.set_tree(solver, root);
    IKAPI.solver.rebuild(solver);
    return solver;
}

static void BM_rebuild_tree(State& state)
{
    ik_solver_t* solver = IKAPI.solver.create(IK_FABRIK);
    ik_node_t* root = create_tree(solver, (Type)state.range(0));
    IKAPI.solver.set_tree(solver, root);

    while (state.KeepRunning())
        IKAPI.solver.rebuild(solver);

    IKAPI.solver.destroy(solver);
}
BENCHMARK(BM_rebuild_tree)
    ->Arg(CHAIN_10)
    ->Arg(TWO_ARMS)
    ->Arg(BINARY_TREE)
    ;

static void BM_FABRIK_solve(State& state)
{
    ik_solver_t* solver = create_solver((Type)state.range(0));

    while (state.KeepRunning())
    {
        /*ik_solver_reset_to_original_pose(solver);*/
        IKAPI.solver.solve(solver);
    }

    IKAPI.solver.destroy(solver);
}
BENCHMARK(BM_FABRIK_solve)
    ->Arg(CHAIN_10)
    ->Arg(TWO_ARMS)
    ->Arg(BINARY_TREE)
    ;

static void BM_FABRIK_solve_final_rotations(State& state)
{
    ik_solver_t* solver = create_solver((Type)state.range(0));

    while (state.KeepRunning())
    {
        /*ik_solver_reset_to_original_pose(solver);*/
        IKAPI.solver.solve(solver);
    }

    IKAPI.solver.destroy(solver);
}
BENCHMARK(BM_FABRIK_solve_final_rotations)
    ->Arg(CHAIN_10)
    ->Arg(TWO_ARMS)
    ->Arg(BINARY_TREE)
    ;

