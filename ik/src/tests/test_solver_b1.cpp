#include "gmock/gmock.h"
#include "ik/node.h"
#include "ik/solver.h"
#include "ik/algorithm.h"
#include "ik/effector.h"
#include "ik/cpputils.hpp"

#define NAME solver_b1

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

TEST_F(NAME, rotate_90_degrees)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(root->position.f, 3, 4, 5);
    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, -2, 2, 0);
    ik_vec3_set(e->target_position.f, 6, 7, 1);

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_VEC3_EQ(base->position, 1, 2, 1);

    // Tip position should remain the same in local space
    EXPECT_VEC3_EQ(tip->position, -2, 2, 0);

    // Base should have rotated about the Z axis by 90°
    EXPECT_QUAT_EQ(base->rotation, 0, 0, -1.0/sqrt(2), 1.0/sqrt(2));

    // Tip rotation should remain identical
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, rotate_90_degrees_no_joint_rotations)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, -2, 2, 0);
    ik_vec3_set(e->target_position.f, 6, 7, 1);
    a->features &= ~IK_ALGORITHM_JOINT_ROTATIONS;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_VEC3_EQ(base->position, 1, 2, 1);
    EXPECT_VEC3_EQ(tip->position, 2, 2, 0);
    EXPECT_QUAT_EQ(base->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, rotate_90_degrees_keep_effector_orientation)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, -2, 2, 0);
    ik_vec3_set(e->target_position.f, 6, 7, 1);
    e->features |= IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_VEC3_EQ(base->position, 1, 2, 1);

    // Tip position should remain the same in local space
    EXPECT_VEC3_EQ(tip->position, -2, 2, 0);

    // Base should have rotated about the Z axis by 90°
    EXPECT_QUAT_EQ(base->rotation, 0, 0, -1.0/sqrt(2), 1.0/sqrt(2));

    // Tip rotation should be 90° about the Z axis in the opposite direction
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 1.0/sqrt(2), 1.0/sqrt(2));
}

TEST_F(NAME, rotate_90_degrees_keep_effector_orientation_no_join_rotations)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, -2, 2, 0);
    ik_vec3_set(e->target_position.f, 6, 7, 1);
    e->features |= IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION;
    a->features &= ~IK_ALGORITHM_JOINT_ROTATIONS;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_VEC3_EQ(base->position, 1, 2, 1);
    EXPECT_VEC3_EQ(tip->position, 2, 2, 0);
    EXPECT_QUAT_EQ(base->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 1.0/sqrt(2), 1.0/sqrt(2));
}

TEST_F(NAME, already_pointing_at_target)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, 2, 2, 0);
    ik_vec3_set(e->target_position.f, 6, 7, 1);

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    const double epsilon = 1e-7;
    EXPECT_VEC3_NEAR(base->position, 1, 2, 1, epsilon);
    EXPECT_VEC3_NEAR(tip->position, 2, 2, 0, epsilon);
    EXPECT_QUAT_NEAR(base->rotation, 0, 0, 0, 1, epsilon);
    EXPECT_QUAT_NEAR(tip->rotation, 0, 0, 0, 1, epsilon);
}

TEST_F(NAME, rotate_90_degrees_initial_rotations)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_quat_set_axis_angle(base->rotation.f, 0, 0, 1, M_PI/4);
    ik_vec3_set(tip->position.f, sqrt(2), sqrt(2), 0);
    ik_quat_set_axis_angle(tip->rotation.f, 0, 0, 1, M_PI/4);
    ik_vec3_set(e->target_position.f, 0, -sqrt(8), 0);

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_VEC3_EQ(base->position, 1, 2, 1);

    // Tip position should remain the same in local space
    EXPECT_VEC3_EQ(tip->position, 2, 2, 0);

    // Base should have rotated about the Z axis by 90°
    EXPECT_QUAT_EQ(base->rotation, 0, 0, 1, M_PI/4 - M_PI/2);

    // Tip rotation should remain identical
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 1, M_PI/4);
}

TEST_F(NAME, rotate_90_degrees_no_joint_rotations_initial_rotations)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, -2, 2, 0);
    ik_vec3_set(e->target_position.f, 7, 3, 0);
    a->features &= ~IK_ALGORITHM_JOINT_ROTATIONS;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_VEC3_EQ(base->position, 1, 2, 1);
    EXPECT_VEC3_EQ(tip->position, 2, 2, 0);
    EXPECT_QUAT_EQ(base->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, rotate_90_degrees_keep_effector_orientation_initial_rotations)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, -2, 2, 0);
    ik_vec3_set(e->target_position.f, 7, 3, 0);
    e->features |= IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_VEC3_EQ(base->position, 1, 2, 1);

    // Tip position should remain the same in local space
    EXPECT_VEC3_EQ(tip->position, -2, 2, 0);

    // Base should have rotated about the Z axis by 90°
    EXPECT_QUAT_EQ(base->rotation, 0, 0, -1.0/sqrt(2), 1.0/sqrt(2));

    // Tip rotation should be 90° about the Z axis in the opposite direction
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 1.0/sqrt(2), 1.0/sqrt(2));
}

TEST_F(NAME, rotate_90_degrees_keep_effector_orientation_no_join_rotations_initial_rotations)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, -2, 2, 0);
    ik_vec3_set(e->target_position.f, 7, 3, 0);
    e->features |= IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION;
    a->features &= ~IK_ALGORITHM_JOINT_ROTATIONS;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_VEC3_EQ(base->position, 1, 2, 1);
    EXPECT_VEC3_EQ(tip->position, 2, 2, 0);
    EXPECT_QUAT_EQ(base->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 1.0/sqrt(2), 1.0/sqrt(2));
}

TEST_F(NAME, already_pointing_at_target_initial_rotations)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, 2, 2, 0);
    ik_vec3_set(e->target_position.f, 3, 3, 0);

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    const double epsilon = 1e-7;
    EXPECT_VEC3_NEAR(base->position, 1, 2, 1, epsilon);
    EXPECT_VEC3_NEAR(tip->position, 2, 2, 0, epsilon);
    EXPECT_QUAT_NEAR(base->rotation, 0, 0, 0, 1, epsilon);
    EXPECT_QUAT_NEAR(tip->rotation, 0, 0, 0, 1, epsilon);
}

TEST_F(NAME, rotate_90_degrees_with_constraint)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> base = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);
    ik::Ref<ik_constraint> c = ik_node_create_constraint(tip);

    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(tip->position.f, 0, 2, 0);
    ik_vec3_set(e->target_position.f, 5, -2, 0);
    ik_constraint_set_type(c, IK_CONSTRAINT_CONE);
    ik_vec3_set(c->data.cone.center.f, 0, 1, 0);
    c->data.cone.max_angle = M_PI / 3;  /* 60° */
    a->features |= IK_ALGORITHM_CONSTRAINTS;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_VEC3_EQ(base->position, 1, 2, 1);
    EXPECT_VEC3_EQ(tip->position, 0, 2, 0);

    // 60° rotation clockwise
    EXPECT_QUAT_EQ(base->rotation, 0, 0, -0.5, 1.0/sqrt(3));

    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}
