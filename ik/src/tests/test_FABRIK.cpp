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

TEST_F(NAME, two_bone_reach_90_degree_to_right)
{
    /*
     *    o
     *    |
     *    o   x   --->  o---o
     *    |             |
     *    o             o
     */
    ik_node_t* root = solver->node->create(0);
    ik_node_t* mid = solver->node->create_child(root, 1);
    ik_node_t* tip = solver->node->create_child(mid, 2);
    IKAPI.vec3.set(mid->position.f, 0, 1, 0);
    IKAPI.vec3.set(tip->position.f, 0, 1, 0);

    ik_effector_t* eff = solver->effector->create();
    IKAPI.vec3.set(eff->target_position.f, 1, 0, 0);
    solver->effector->attach(eff, tip);

    ik.solver.set_tree(solver, root);
    ik.solver.rebuild(solver);
    ik.solver.solve(solver);

    const double error = 1e-15;
    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));

    EXPECT_THAT(mid->position.x, DoubleNear(0, error));
    EXPECT_THAT(mid->position.y, DoubleNear(1, error));
    EXPECT_THAT(mid->position.z, DoubleNear(0, error));

    EXPECT_THAT(tip->position.x, DoubleNear(1, error));
    EXPECT_THAT(tip->position.y, DoubleNear(0, error));
    EXPECT_THAT(tip->position.z, DoubleNear(0, error));
}
