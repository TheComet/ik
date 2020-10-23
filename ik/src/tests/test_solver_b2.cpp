#include "gmock/gmock.h"
#include "ik/bone.h"
#include "ik/solver.h"
#include "ik/algorithm.h"
#include "ik/effector.h"
#include "ik/vec3.inl"
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

TEST_F(NAME, reach_target_colinear_segments)
{
    ik::Ref<ik_bone> root = ik_bone_create();
    ik::Ref<ik_bone> base = ik_bone_create_child(root);
    ik::Ref<ik_bone> mid = ik_bone_create_child(base);
    ik::Ref<ik_bone> tip = ik_bone_create_child(mid);
    ik::Ref<ik_effector> e = ik_bone_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(base, IK_TWO_BONE);

    ik_vec3_set(root->position.f, 3, 4, 5);
    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(mid->position.f, 0, 2, 0);
    ik_vec3_set(tip->position.f, 0, 2, 0);
    ik_vec3_set(e->target_position.f, 2, 0, 0);

    // This should form the 3 bones into an equilateral triangle
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_VEC3_EQ(base->position, 1, 2, 1);

    // Mid position should remain the same in local space
    EXPECT_VEC3_EQ(mid->position, 0, 2, 0);

    // Tip position should remain the same in local space
    EXPECT_VEC3_EQ(tip->position, 0, 2, 0);

    // Base should have rotated about the Z axis by 60°
    EXPECT_QUAT_EQ(base->rotation, 0, 0, -0.5, 1.0/sqrt(3));

    // Mid should have rotated about the Z axis by 60°
    EXPECT_QUAT_EQ(mid->rotation, 0, 0, -0.5, 1.0/sqrt(3));

    // Tip rotation should remain identical
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, reach_target_coplanar_segments)
{
    ik::Ref<ik_bone> root = ik_bone_create();
    ik::Ref<ik_bone> base = ik_bone_create_child(root);
    ik::Ref<ik_bone> mid = ik_bone_create_child(base);
    ik::Ref<ik_bone> tip = ik_bone_create_child(mid);
    ik::Ref<ik_effector> e = ik_bone_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(base, IK_TWO_BONE);

    ik_vec3_set(root->position.f, 3, 4, 5);
    ik_vec3_set(base->position.f, 1, 2, 1);
    ik_vec3_set(mid->position.f, 0, 2, 0);
    ik_vec3_set(tip->position.f, 2, 0, 0);
    ik_vec3_set(e->target_position.f, 2, 0, 0);

    // This should form the 3 bones into an equilateral triangle
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_VEC3_EQ(base->position, 1, 2, 1);

    // Mid position should remain the same in local space
    EXPECT_VEC3_EQ(mid->position, 0, 2, 0);

    // Tip position should remain the same in local space
    EXPECT_VEC3_EQ(tip->position, 2, 0, 0);

    // Base should have rotated about the Z axis by 60°
    EXPECT_QUAT_EQ(base->rotation, 0, 0, -0.5, 1.0/sqrt(3));

    // Mid should have rotated about the Z axis by 60°
    EXPECT_QUAT_EQ(mid->rotation, 0, 0, -0.5, 1.0/sqrt(3));

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

