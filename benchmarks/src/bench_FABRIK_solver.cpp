#include "benchmark/benchmark.h"
#include "ik/solver.h"
#include "ik/node.h"
#include "ik/effector.h"
#include "ik/chain.h"

using namespace benchmark;

enum Type
{
    CHAIN_10,
    TWO_ARMS,
    BINARY_TREE
};

static void build_tree_long_chains(ik_node_t* parent, int depth, int* guid)
{
    ik_node_t* child1 = ik_node_create((*guid)++); child1->initial_position.v.x = (depth*7) + 1; child1->initial_position.v.y = (depth*7) + 1;
    ik_node_t* child2 = ik_node_create((*guid)++); child1->initial_position.v.x = (depth*7) + 2; child1->initial_position.v.y = (depth*7) + 2;
    ik_node_t* child3 = ik_node_create((*guid)++); child1->initial_position.v.x = (depth*7) + 3; child1->initial_position.v.y = (depth*7) + 3;
    ik_node_t* child4 = ik_node_create((*guid)++); child1->initial_position.v.x = (depth*7) + 4; child1->initial_position.v.y = (depth*7) + 4;
    ik_node_t* child5 = ik_node_create((*guid)++); child1->initial_position.v.x = (depth*7) + 5; child1->initial_position.v.y = (depth*7) + 5;
    ik_node_t* child6 = ik_node_create((*guid)++); child1->initial_position.v.x = (depth*7) + 6; child1->initial_position.v.y = (depth*7) + 6;
    ik_node_add_child(parent, child1);
    ik_node_add_child(child1, child2);
    ik_node_add_child(child2, child3);

    ik_node_add_child(parent, child4);
    ik_node_add_child(child4, child5);
    ik_node_add_child(child5, child6);

    if(depth > 0)
    {
        build_tree_long_chains(child3, depth-1, guid);
        build_tree_long_chains(child6, depth-1, guid);
    }
    else
    {
        ik_effector_t* eff1 = ik_effector_create();
        ik_effector_t* eff2 = ik_effector_create();
        eff1->target_position.v.x = depth * 100;
        eff2->target_position.v.z = depth * 100;
        ik_node_attach_effector(child3, eff1);
        ik_node_attach_effector(child6, eff2);
    }
}

static ik_node_t* create_tree(Type type)
{
    static int guid = 0;
    ik_node_t* root = ik_node_create(guid++);

    switch (type)
    {
        case CHAIN_10:
        {
            ik_node_t* parent = root;
            for (int i = 0; i != 10; ++i)
            {
                ik_node_t* child = ik_node_create(guid++);
                child->initial_position.v.y = i + 1; // 1 unit higher every time
                ik_node_add_child(parent, child);
                parent = child;
            }
            ik_effector_t* eff = ik_effector_create();
            eff->target_position.v.x = 5;
            eff->target_position.v.y = 0;
            ik_node_attach_effector(parent, eff);
        } break;

        case TWO_ARMS:
        {
            ik_node_t* child1 = ik_node_create(guid++); child1->initial_position.v.y = 1;
            ik_node_t* child2 = ik_node_create(guid++); child2->initial_position.v.y = 2;
            ik_node_t* child3 = ik_node_create(guid++); child3->initial_position.v.y = 3;
            ik_node_add_child(root, child1);
            ik_node_add_child(child1, child2);
            ik_node_add_child(child2, child3);

            ik_node_t* sub_base = child3;

            child1 = ik_node_create(guid++); child1->initial_position.v.y = 4; child1->initial_position.v.x = -1;
            child2 = ik_node_create(guid++); child2->initial_position.v.y = 5; child2->initial_position.v.x = -2;
            child3 = ik_node_create(guid++); child2->initial_position.v.y = 6; child2->initial_position.v.x = -3;
            ik_node_add_child(sub_base, child1);
            ik_node_add_child(child1, child2);
            ik_node_add_child(child2, child3);

            ik_effector_t* eff = ik_effector_create();
            eff->target_position.v.z = 2; // make it grab forwards
            ik_node_attach_effector(child3, eff);

            child1 = ik_node_create(guid++); child1->initial_position.v.y = 4; child1->initial_position.v.x = 1;
            child2 = ik_node_create(guid++); child2->initial_position.v.y = 5; child2->initial_position.v.x = 2;
            child3 = ik_node_create(guid++); child2->initial_position.v.y = 6; child2->initial_position.v.x = 3;
            ik_node_add_child(sub_base, child1);
            ik_node_add_child(child1, child2);
            ik_node_add_child(child2, child3);

            eff = ik_effector_create();
            eff->target_position.v.z = 2; // make it grab forwards
            ik_node_attach_effector(child3, eff);
        } break;

        case BINARY_TREE:
        {
            build_tree_long_chains(root, 10, &guid);
        } break;
    };

    return root;
}

static ik_solver_t* create_solver(Type type)
{
    ik_solver_t* solver = ik_solver_create(SOLVER_FABRIK);
    ik_node_t* root = create_tree(type);
    ik_solver_set_tree(solver, root);
    ik_solver_rebuild_data(solver);
    return solver;
}

static void BM_rebuild_tree(State& state)
{
    ik_solver_t* solver = ik_solver_create(SOLVER_FABRIK);
    ik_node_t* root = create_tree((Type)state.range(0));
    ik_solver_set_tree(solver, root);

    while (state.KeepRunning())
        ik_solver_rebuild_data(solver);

    ik_solver_destroy(solver);
}
BENCHMARK(BM_rebuild_tree)
    ->Arg(CHAIN_10)
    ->Arg(TWO_ARMS)
    ->Arg(BINARY_TREE)
    ;

static void BM_FABRIK_reset_tree(State& state)
{
    ik_solver_t* solver = create_solver((Type)state.range(0));

    while (state.KeepRunning())
        ik_solver_reset_solved_data(solver);

    ik_solver_destroy(solver);
}
BENCHMARK(BM_FABRIK_reset_tree)
    ->Arg(CHAIN_10)
    ->Arg(TWO_ARMS)
    ->Arg(BINARY_TREE)
    ;

static void BM_FABRIK_solve(State& state)
{
    ik_solver_t* solver = create_solver((Type)state.range(0));

    while (state.KeepRunning())
    {
        ik_solver_reset_solved_data(solver);
        ik_solver_solve(solver);
    }

    ik_solver_destroy(solver);
}
BENCHMARK(BM_FABRIK_solve)
    ->Arg(CHAIN_10)
    ->Arg(TWO_ARMS)
    ->Arg(BINARY_TREE)
    ;

static void BM_FABRIK_solve_final_rotations(State& state)
{
    ik_solver_t* solver = create_solver((Type)state.range(0));
    solver->flags |= SOLVER_CALCULATE_FINAL_ROTATIONS;

    while (state.KeepRunning())
    {
        ik_solver_reset_solved_data(solver);
        ik_solver_solve(solver);
    }

    ik_solver_destroy(solver);
}
BENCHMARK(BM_FABRIK_solve_final_rotations)
    ->Arg(CHAIN_10)
    ->Arg(TWO_ARMS)
    ->Arg(BINARY_TREE)
    ;

