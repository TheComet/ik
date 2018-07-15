#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME FABRIK

using namespace ::testing;

class NAME : public Test
{
public:
    NAME() : solver(NULL) {}

    virtual void SetUp()
    {
        solver = IKAPI.solver.create(IK_FABRIK);
    }

    virtual void TearDown()
    {
        IKAPI.solver.destroy(solver);
    }

protected:
    ik_solver_t* solver;
};

static void buildTreeLongChains(ik_solver_t* solver, ik_node_t* parent, int depth, int* guid)
{
    ik_node_t* child1 = solver->node->create(++(*guid));
    ik_node_t* child2 = solver->node->create(++(*guid));
    ik_node_t* child3 = solver->node->create(++(*guid));
    ik_node_t* child4 = solver->node->create(++(*guid));
    ik_node_t* child5 = solver->node->create(++(*guid));
    ik_node_t* child6 = solver->node->create(++(*guid));
    solver->node->add_child(parent, child1);
    solver->node->add_child(child1, child2);
    solver->node->add_child(child2, child3);

    solver->node->add_child(parent, child4);
    solver->node->add_child(child4, child5);
    solver->node->add_child(child5, child6);

    if(depth < 4)
    {
        buildTreeLongChains(solver, child3, depth+1, guid);
        buildTreeLongChains(solver, child6, depth+1, guid);
    }
    else
    {
        ik_effector_t* eff1 = solver->effector->create();
        ik_effector_t* eff2 = solver->effector->create();
        solver->effector->attach(eff1, child3);
        solver->effector->attach(eff2, child6);
    }
}

static void buildTreeShortChains(ik_solver_t* solver, ik_node_t* parent, int depth, int* guid)
{
    ik_node_t* child1 = solver->node->create(++(*guid));
    ik_node_t* child2 = solver->node->create(++(*guid));
    solver->node->add_child(parent, child1);
    solver->node->add_child(parent, child2);

    if(depth < 4)
    {
        buildTreeLongChains(solver, child1, depth+1, guid);
        buildTreeLongChains(solver, child2, depth+1, guid);
    }
    else
    {
        ik_effector_t* eff1 = solver->effector->create();
        ik_effector_t* eff2 = solver->effector->create();
        solver->effector->attach(eff1, child1);
        solver->effector->attach(eff2, child2);
    }
}


TEST_F(NAME, binary_tree_with_long_chains)
{
    int guid = 0;
    ik_node_t* root = solver->node->create(0);
    buildTreeLongChains(solver, root, 0, &guid);

    IKAPI.solver.set_tree(solver, root);
    IKAPI.solver.rebuild(solver);
    IKAPI.solver.solve(solver);
}

TEST_F(NAME, binary_tree_with_short_chains)
{
    int guid = 0;
    ik_node_t* root = solver->node->create(0);
    buildTreeShortChains(solver, root, 0, &guid);

    IKAPI.solver.set_tree(solver, root);
    IKAPI.solver.rebuild(solver);
    IKAPI.solver.solve(solver);
}
