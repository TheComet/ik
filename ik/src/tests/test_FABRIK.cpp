#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME FABRIK

using namespace ::testing;

const double pi = std::atan(1.0) * 4;

class NAME : public Test
{
public:
    NAME() : solver(NULL) {}

    virtual void SetUp()
    {
        solver = ik_solver_create(IK_FABRIK);
    }

    virtual void TearDown()
    {
        ik_solver_destroy(solver);
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

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild(solver);
    ik_solver_solve(solver);
}

TEST_F(NAME, binary_tree_with_short_chains)
{
    int guid = 0;
    ik_node_t* root = solver->node->create(0);
    buildTreeShortChains(solver, root, 0, &guid);

    ik_solver_set_tree(solver, root);
    ik_solver_rebuild(solver);
    ik_solver_solve(solver);
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
    ik_vec3_set(mid->position.f, 0, 2, 0);
    ik_vec3_set(tip->position.f, 0, 3, 0);

    ik_effector_t* eff = solver->effector->create();
    ik_vec3_set(eff->target_position.f, 3, 2, 0);
    solver->effector->attach(eff, tip);

    ik.solver.set_tree(solver, root);
    ik.solver.rebuild(solver);
    ik.solver.solve(solver);

    const double error = solver->tolerance;
    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));

    EXPECT_THAT(mid->position.x, DoubleNear(0, error));
    EXPECT_THAT(mid->position.y, DoubleNear(2, error));
    EXPECT_THAT(mid->position.z, DoubleNear(0, error));

    EXPECT_THAT(tip->position.x, DoubleNear(0, error));
    EXPECT_THAT(tip->position.y, DoubleNear(3, error));
    EXPECT_THAT(tip->position.z, DoubleNear(0, error));

    EXPECT_THAT(root->rotation.w, DoubleNear(1, error));
    EXPECT_THAT(root->rotation.x, DoubleNear(0, error));
    EXPECT_THAT(root->rotation.y, DoubleNear(0, error));
    EXPECT_THAT(root->rotation.z, DoubleNear(0, error));

    EXPECT_THAT(mid->rotation.w, DoubleNear(1/sqrt(2), error));
    EXPECT_THAT(mid->rotation.x, DoubleNear(0, error));
    EXPECT_THAT(mid->rotation.y, DoubleNear(0, error));
    EXPECT_THAT(mid->rotation.z, DoubleNear(1/sqrt(2), error));

    EXPECT_THAT(tip->rotation.w, DoubleNear(1, error));
    EXPECT_THAT(tip->rotation.x, DoubleNear(0, error));
    EXPECT_THAT(tip->rotation.y, DoubleNear(0, error));
    EXPECT_THAT(tip->rotation.z, DoubleNear(0, error));
}

TEST_F(NAME, two_bone_reach_90_degree_to_right_with_rest_pose_rotations)
{
    /*
     *      o
     *     /
     *    o   x   --->  o---o
     *    |             |
     *    o             o
     */
    ik_node_t* root = solver->node->create(0);
    ik_node_t* mid = solver->node->create_child(root, 1);
    ik_node_t* tip = solver->node->create_child(mid, 2);
    ik_vec3_set(mid->position.f, 0, 2, 0);
    ik_vec3_set(tip->position.f, 0, 3, 0);
    ik_quat_set_axis_angle(root->rotation.f, 0, 0, 1, 45*pi/180);
    ik_quat_set_axis_angle(mid->rotation.f, 0, 0, 1, 45*pi/180);

    ik_effector_t* eff = solver->effector->create();
    ik_vec3_set(eff->target_position.f, 3, 2, 0);
    solver->effector->attach(eff, tip);

    ik.solver.set_tree(solver, root);
    ik.solver.rebuild(solver);
    ik.solver.solve(solver);

    const double error = solver->tolerance;
    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));

    EXPECT_THAT(mid->position.x, DoubleNear(0, error));
    EXPECT_THAT(mid->position.y, DoubleNear(2, error));
    EXPECT_THAT(mid->position.z, DoubleNear(0, error));

    EXPECT_THAT(tip->position.x, DoubleNear(0, error));
    EXPECT_THAT(tip->position.y, DoubleNear(3, error));
    EXPECT_THAT(tip->position.z, DoubleNear(0, error));

    EXPECT_THAT(root->rotation.w, DoubleNear(1, error));
    EXPECT_THAT(root->rotation.x, DoubleNear(0, error));
    EXPECT_THAT(root->rotation.y, DoubleNear(0, error));
    EXPECT_THAT(root->rotation.z, DoubleNear(0, error));

    EXPECT_THAT(mid->rotation.w, DoubleNear(1/sqrt(2), error));
    EXPECT_THAT(mid->rotation.x, DoubleNear(0, error));
    EXPECT_THAT(mid->rotation.y, DoubleNear(0, error));
    EXPECT_THAT(mid->rotation.z, DoubleNear(1/sqrt(2), error));

    EXPECT_THAT(tip->rotation.w, DoubleNear(1, error));
    EXPECT_THAT(tip->rotation.x, DoubleNear(0, error));
    EXPECT_THAT(tip->rotation.y, DoubleNear(0, error));
    EXPECT_THAT(tip->rotation.z, DoubleNear(0, error));
}

TEST_F(NAME, two_bone_reach_90_degree_to_right_with_rest_pose_translations)
{
    /*
     *      o
     *     /
     *    o   x   --->  o---o
     *    |             |
     *    o             o
     */
    ik_node_t* root = solver->node->create(0);
    ik_node_t* mid = solver->node->create_child(root, 1);
    ik_node_t* tip = solver->node->create_child(mid, 2);
    ik_vec3.set(mid->position_f, 2/sqrt(2), 2/sqrt(2), 0);
    ik_vec3.set(tip->position_f, 3/sqrt(2), 3/sqrt(2), 0);

    ik_effector_t* eff = solver->effector->create();
    ik_vec3_set(eff->target_position.f, 3, 2, 0);
    solver->effector->attach(eff, tip);

    ik.solver.set_tree(solver, root);
    ik.solver.rebuild(solver);
    ik.solver.solve(solver);

    ik_quat_t expected_root, expected_mid;
    ik_quat_set_axis_angle(expected_root.f, 0, 0, 1, -45*pi/180);
    ik_quat_set_axis_angle(expected_mid.f, 0, 0, 1, 45*pi/180);

    const double error = solver->tolerance;
    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));

    EXPECT_THAT(mid->position.x, DoubleNear(2/sqrt(2), error));
    EXPECT_THAT(mid->position.y, DoubleNear(2/sqrt(2), error));
    EXPECT_THAT(mid->position.z, DoubleNear(0, error));

    EXPECT_THAT(tip->position.x, DoubleNear(3/sqrt(2), error));
    EXPECT_THAT(tip->position.y, DoubleNear(3/sqrt(2), error));
    EXPECT_THAT(tip->position.z, DoubleNear(0, error));

    EXPECT_THAT(root->rotation.w, DoubleNear(expected_root.w, error));
    EXPECT_THAT(root->rotation.x, DoubleNear(expected_root.x, error));
    EXPECT_THAT(root->rotation.y, DoubleNear(expected_root.y, error));
    EXPECT_THAT(root->rotation.z, DoubleNear(expected_root.z, error));

    EXPECT_THAT(mid->rotation.w, DoubleNear(expected_mid.w, error));
    EXPECT_THAT(mid->rotation.x, DoubleNear(expected_mid.x, error));
    EXPECT_THAT(mid->rotation.y, DoubleNear(expected_mid.y, error));
    EXPECT_THAT(mid->rotation.z, DoubleNear(expected_mid.z, error));

    EXPECT_THAT(tip->rotation.w, DoubleNear(1, error));
    EXPECT_THAT(tip->rotation.x, DoubleNear(0, error));
    EXPECT_THAT(tip->rotation.y, DoubleNear(0, error));
    EXPECT_THAT(tip->rotation.z, DoubleNear(0, error));
}
