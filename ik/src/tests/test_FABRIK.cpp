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
        solver = IKAPI.solver.create(IKAPI.solver.FABRIK);
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
    ik_node_t* child1 = IKAPI.node.create(++(*guid));
    ik_node_t* child2 = IKAPI.node.create(++(*guid));
    ik_node_t* child3 = IKAPI.node.create(++(*guid));
    ik_node_t* child4 = IKAPI.node.create(++(*guid));
    ik_node_t* child5 = IKAPI.node.create(++(*guid));
    ik_node_t* child6 = IKAPI.node.create(++(*guid));
    IKAPI.node.add_child(parent, child1);
    IKAPI.node.add_child(child1, child2);
    IKAPI.node.add_child(child2, child3);

    IKAPI.node.add_child(parent, child4);
    IKAPI.node.add_child(child4, child5);
    IKAPI.node.add_child(child5, child6);

    if(depth < 4)
    {
        buildTreeLongChains(solver, child3, depth+1, guid);
        buildTreeLongChains(solver, child6, depth+1, guid);
    }
    else
    {
        ik_effector_t* eff1 = IKAPI.effector.create();
        ik_effector_t* eff2 = IKAPI.effector.create();
        IKAPI.effector.attach(eff1, child3);
        IKAPI.effector.attach(eff2, child6);
    }
}

static void buildTreeShortChains(ik_solver_t* solver, ik_node_t* parent, int depth, int* guid)
{
    ik_node_t* child1 = IKAPI.node.create(++(*guid));
    ik_node_t* child2 = IKAPI.node.create(++(*guid));
    IKAPI.node.add_child(parent, child1);
    IKAPI.node.add_child(parent, child2);

    if(depth < 4)
    {
        buildTreeLongChains(solver, child1, depth+1, guid);
        buildTreeLongChains(solver, child2, depth+1, guid);
    }
    else
    {
        ik_effector_t* eff1 = IKAPI.effector.create();
        ik_effector_t* eff2 = IKAPI.effector.create();
        IKAPI.effector.attach(eff1, child1);
        IKAPI.effector.attach(eff2, child2);
    }
}


TEST_F(NAME, binary_tree_with_long_chains)
{
    int guid = 0;
    ik_node_t* root = IKAPI.node.create(0);
    buildTreeLongChains(solver, root, 0, &guid);

    IKAPI.solver.set_tree(solver, root);
    IKAPI.solver.rebuild(solver);
    IKAPI.solver.solve(solver);
}

TEST_F(NAME, binary_tree_with_short_chains)
{
    int guid = 0;
    ik_node_t* root = IKAPI.node.create(0);
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
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* mid = IKAPI.node.create_child(root, 1);
    ik_node_t* tip = IKAPI.node.create_child(mid, 2);
    IKAPI.vec3.set(mid->position.f, 0, 2, 0);
    IKAPI.vec3.set(tip->position.f, 0, 3, 0);

    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.vec3.set(eff->target_position.f, 3, 2, 0);
    IKAPI.effector.attach(eff, tip);

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
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* mid = IKAPI.node.create_child(root, 1);
    ik_node_t* tip = IKAPI.node.create_child(mid, 2);
    IKAPI.vec3.set(mid->position.f, 0, 2, 0);
    IKAPI.vec3.set(tip->position.f, 0, 3, 0);
    IKAPI.quat.set_axis_angle(root->rotation.f, 0, 0, 1, 45*pi/180);
    IKAPI.quat.set_axis_angle(mid->rotation.f, 0, 0, 1, 45*pi/180);

    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.vec3.set(eff->target_position.f, 3, 2, 0);
    IKAPI.effector.attach(eff, tip);

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
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* mid = IKAPI.node.create_child(root, 1);
    ik_node_t* tip = IKAPI.node.create_child(mid, 2);
    IKAPI.vec3.set(mid->position.f, 2/sqrt(2), 2/sqrt(2), 0);
    IKAPI.vec3.set(tip->position.f, 3/sqrt(2), 3/sqrt(2), 0);

    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.vec3.set(eff->target_position.f, 3, 2, 0);
    IKAPI.effector.attach(eff, tip);

    ik.solver.set_tree(solver, root);
    ik.solver.rebuild(solver);
    ik.solver.solve(solver);

    ik_quat_t expected_root, expected_mid;
    IKAPI.quat.set_axis_angle(expected_root.f, 0, 0, 1, -45*pi/180);
    IKAPI.quat.set_axis_angle(expected_mid.f, 0, 0, 1, 45*pi/180);

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
