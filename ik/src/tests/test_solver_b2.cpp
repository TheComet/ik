#include "gmock/gmock.h"
#include "ik/bone.h"
#include "ik/solver.h"
#include "ik/algorithm.h"
#include "ik/effector.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"
#include "ik/cpputils.hpp"

#define NAME solver_b2

using namespace ::testing;

class NAME : public Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

protected:
};

#define EXPECT_VEC3_EQ(vec, vx, vy, vz) do {                                  \
        EXPECT_THAT(vec.v.x, DoubleEq(vx));                                   \
        EXPECT_THAT(vec.v.y, DoubleEq(vy));                                   \
        EXPECT_THAT(vec.v.z, DoubleEq(vz));                                   \
    } while(0)

#define EXPECT_VEC3_NEAR(vec, vx, vy, vz, epsilon) do {                       \
        EXPECT_THAT(vec.v.x, DoubleNear(vx, epsilon));                        \
        EXPECT_THAT(vec.v.y, DoubleNear(vy, epsilon));                        \
        EXPECT_THAT(vec.v.z, DoubleNear(vz, epsilon));                        \
    } while(0)

#define EXPECT_QUAT_EQ(quat, qx, qy, qz, qw) do {                             \
        EXPECT_THAT(quat.q.x, DoubleEq(qx));                                  \
        EXPECT_THAT(quat.q.y, DoubleEq(qy));                                  \
        EXPECT_THAT(quat.q.z, DoubleEq(qz));                                  \
        EXPECT_THAT(quat.q.w, DoubleEq(qw));                                  \
    } while(0)

#define EXPECT_QUAT_NEAR(quat, qx, qy, qz, qw, epsilon) do {                  \
        EXPECT_THAT(quat.q.x, DoubleNear(qx, epsilon));                       \
        EXPECT_THAT(quat.q.y, DoubleNear(qy, epsilon));                       \
        EXPECT_THAT(quat.q.z, DoubleNear(qz, epsilon));                       \
        EXPECT_THAT(quat.q.w, DoubleNear(qw, epsilon));                       \
    } while(0)

TEST_F(NAME, unreachable_1)
{
    ik::Ref<ik_bone> base = ik_bone_create();
    ik::Ref<ik_bone> tip = ik_bone_create_child(base);
    base->length = 0.2;  ik_quat_set_axis_angle(base->rotation.f, 1, 0, 0, M_PI/4);
    tip->length = 0.3; ik_vec3_set(tip->position.f, 0, 0.2, 0);

    ik::Ref<ik_effector> e = ik_bone_create_effector(tip);
    ik_vec3_set(e->target_position.f, 0, 0, 10);

    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(base, IK_TWO_BONE);
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);
}

TEST_F(NAME, reachable_1)
{
    ik::Ref<ik_bone> base = ik_bone_create();
    ik::Ref<ik_bone> tip = ik_bone_create_child(base);
    ik_quat_set_axis_angle(base->rotation.f, 1, 0, 0, M_PI/4);
    base->length = 1;
    tip->length = 1;

    ik::Ref<ik_effector> e = ik_bone_create_effector(tip);
    ik_vec3_set(e->target_position.f, 0, 0, sqrt(2));

    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(base, IK_TWO_BONE);
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);
}

TEST_F(NAME, reach_target_colinear_segments)
{
    ik::Ref<ik_bone> root = ik_bone_create();
    ik::Ref<ik_bone> base = ik_bone_create_child(root);
    ik::Ref<ik_bone> tip = ik_bone_create_child(base);
    ik::Ref<ik_effector> e = ik_bone_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(base, IK_TWO_BONE);

    ik_vec3_set(root->position.f, 3, 4, 5);
    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, 0, 2, 0);
    ik_vec3_set(e->target_position.f, 2, 0, 0);

    // This should form the 3 bones into an equilateral triangle
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_VEC3_EQ(base->position, 1, 2, 1);

    // Tip position should remain the same in local space
    EXPECT_VEC3_EQ(tip->position, 0, 2, 0);

    // Base should have rotated about the Z axis by 60°
    EXPECT_QUAT_EQ(base->rotation, 0, 0, -0.5, 1.0/sqrt(3));

    // Tip rotation should remain identical
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, reach_target_coplanar_segments)
{
    ik::Ref<ik_bone> root = ik_bone_create();
    ik::Ref<ik_bone> base = ik_bone_create_child(root);
    ik::Ref<ik_bone> tip = ik_bone_create_child(base);
    ik::Ref<ik_effector> e = ik_bone_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(base, IK_TWO_BONE);

    ik_vec3_set(root->position.f, 3, 4, 5);
    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, 2, 0, 0);
    ik_vec3_set(e->target_position.f, 2, 0, 0);

    // This should form the 3 bones into an equilateral triangle
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_VEC3_EQ(base->position, 1, 2, 1);

    // Tip position should remain the same in local space
    EXPECT_VEC3_EQ(tip->position, 2, 0, 0);

    // Base should have rotated about the Z axis by 60°
    EXPECT_QUAT_EQ(base->rotation, 0, 0, -0.5, 1.0/sqrt(3));

    // Tip rotation should remain identical
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, reach_target_keep_effector_orientation)
{

}

TEST_F(NAME, reach_target_keep_effector_orientation_no_join_rotations)
{

}

TEST_F(NAME, already_on_target)
{

}

TEST_F(NAME, reach_target_with_constraint)
{

}

#if 0
TEST_F(NAME, reach_target_1)
{
    /*
     * This test sets up a two-bone tree with bones at 135° to one another with
     * lengths of 3. The target is positioned such that the bones will be
     * arranged at 90° when they reach the target.
     *
     * Length of 3 will catch any obvious normalization issues (which don't
     * usually appear with bone lengths of 1) and having it solve in all 3
     * dimensions will catch any obvious transform/rotation issues.
     *
     *   o
     *   |
     *   o      x   --->  o---x
     *    \               |
     *     o              o
     */
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* mid = IKAPI.node.create_child(root, 1);
    ik_node_t* tip = IKAPI.node.create_child(mid, 2);
    IKAPI.vec3.set(mid->position.f, -3/sqrt(3), 3/sqrt(3), -3/sqrt(3));  /* length 3 */
    IKAPI.vec3.set(tip->position.f, 0, 3, 0);

    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.vec3.set(eff->target_position.f, 3/sqrt(2), 3, 3/sqrt(2));
    IKAPI.effector.attach(eff, tip);

    ik.solver.set_tree(solver, root);
    ik.solver.rebuild(solver);
    ik.solver.solve(solver);

    const double error = 1e-15;

    /* Local positions should remain the same, it's the local rotations that
     * should be affected */
    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));
    EXPECT_THAT(mid->position.x, DoubleNear(-3/sqrt(3), error));
    EXPECT_THAT(mid->position.y, DoubleNear(3/sqrt(3), error));
    EXPECT_THAT(mid->position.z, DoubleNear(-3/sqrt(3), error));
    EXPECT_THAT(tip->position.x, DoubleNear(0, error));
    EXPECT_THAT(tip->position.y, DoubleNear(3, error));
    EXPECT_THAT(tip->position.z, DoubleNear(0, error));

    IKAPI.transform.node(IKAPI.solver.get_tree(solver), IKAPI.transform.L2G);

    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));
    EXPECT_THAT(mid->position.x, DoubleNear(0, error));
    EXPECT_THAT(mid->position.y, DoubleNear(3, error));
    EXPECT_THAT(mid->position.z, DoubleNear(0, error));
    EXPECT_THAT(tip->position.x, DoubleNear(3/sqrt(2), error));
    EXPECT_THAT(tip->position.y, DoubleNear(0, error));
    EXPECT_THAT(tip->position.z, DoubleNear(3/sqrt(2), error));
}

TEST_F(NAME, unreachable_1)
{
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* mid = IKAPI.node.create_child(root, 1);
    ik_node_t* tip = IKAPI.node.create_child(mid, 2);
    IKAPI.vec3.set(mid->position.f, 0, 3, 0);
    IKAPI.vec3.set(tip->position.f, 0, 3, 0);

    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.vec3.set(eff->target_position.f, 6.1/sqrt(3), 6.1/sqrt(3), 6.1/sqrt(3));
    IKAPI.effector.attach(eff, tip);

    ik.solver.set_tree(solver, root);
    ik.solver.rebuild(solver);
    ik.solver.solve(solver);

    const double error = 1e-15;

    /* Local positions should remain the same, it's the local rotations that
     * should be affected */
    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));
    EXPECT_THAT(mid->position.x, DoubleNear(-3/sqrt(3), error));
    EXPECT_THAT(mid->position.y, DoubleNear(3/sqrt(3), error));
    EXPECT_THAT(mid->position.z, DoubleNear(-3/sqrt(3), error));
    EXPECT_THAT(tip->position.x, DoubleNear(0, error));
    EXPECT_THAT(tip->position.y, DoubleNear(3, error));
    EXPECT_THAT(tip->position.z, DoubleNear(0, error));

    IKAPI.transform.node(IKAPI.solver.get_tree(solver), IKAPI.transform.L2G);

    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));
    EXPECT_THAT(mid->position.x, DoubleNear(0, error));
    EXPECT_THAT(mid->position.y, DoubleNear(3, error));
    EXPECT_THAT(mid->position.z, DoubleNear(0, error));
    EXPECT_THAT(tip->position.x, DoubleNear(3/sqrt(2), error));
    EXPECT_THAT(tip->position.y, DoubleNear(0, error));
    EXPECT_THAT(tip->position.z, DoubleNear(3/sqrt(2), error));
}
#endif
