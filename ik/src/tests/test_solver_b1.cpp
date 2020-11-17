#include "gmock/gmock.h"

#include "ik/bone.h"
#include "ik/solver.h"
#include "ik/algorithm.h"
#include "ik/effector.h"
#include "ik/vec3.inl"
#include "ik/cpputils.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

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
    ik::Ref<ik_bone> root = ik_bone_create(2);
    ik::Ref<ik_bone> bone = ik_bone_create_child(root, 2);
    ik::Ref<ik_effector> e = ik_bone_create_effector(bone);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(bone, IK_ONE_BONE);

    ik_vec3_set(root->position.f, 0, 4, 5);
    ik_vec3_set(bone->position.f, 0, 2, 1);
    ik_vec3_set(e->target_position.f, 0, 4, 1);

    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    // Base and root position should not change
    EXPECT_VEC3_EQ(root->position, 0, 4, 5);
    EXPECT_VEC3_EQ(bone->position, 0, 2, 1);

    // Base should have rotated about the X axis by -90°
    EXPECT_QUAT_EQ(bone->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2));
}

TEST_F(NAME, already_pointing_at_target)
{
    ik::Ref<ik_bone> root = ik_bone_create(2);
    ik::Ref<ik_bone> bone = ik_bone_create_child(root, 2);
    ik::Ref<ik_effector> e = ik_bone_create_effector(bone);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(bone, IK_ONE_BONE);

    ik_vec3_set(root->position.f, 0, 4, 5);
    ik_vec3_set(bone->position.f, 0, 2, 1);
    ik_vec3_set(e->target_position.f, 0, 2, 3);

    ik::Ref<ik_solver> s = ik_solver_build(bone);
    ik_solver_solve(s);

    const double epsilon = 1e-7;
    EXPECT_VEC3_NEAR(root->position, 0, 4, 5, epsilon);
    EXPECT_VEC3_NEAR(bone->position, 0, 2, 1, epsilon);
    EXPECT_QUAT_NEAR(bone->rotation, 0, 0, 0, 1, epsilon);
}

TEST_F(NAME, rotate_90_degrees_with_constraint)
{
    ik::Ref<ik_bone> root = ik_bone_create(2);
    ik::Ref<ik_bone> bone = ik_bone_create_child(root, 2);
    ik::Ref<ik_effector> e = ik_bone_create_effector(bone);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(bone, IK_ONE_BONE);
    ik::Ref<ik_constraint> c = ik_bone_create_constraint(bone);

    ik_vec3_set(root->position.f, 0, 4, 5);
    ik_vec3_set(bone->position.f, 0, 2, 1);
    ik_vec3_set(e->target_position.f, 0, 2, 3);

    ik_constraint_set_cone(c, 0, 0, 0, 1, 0, M_PI/3.0); /* 0° - 60° */
    a->features |= IK_ALGORITHM_CONSTRAINTS;

    ik::Ref<ik_solver> s = ik_solver_build(bone);
    ik_solver_solve(s);

    EXPECT_VEC3_EQ(root->position, 0, 4, 5);
    EXPECT_VEC3_EQ(bone->position, 0, 2, 1);

    // Expect a -60° rotation instead of -90°, due to cone constraint
    EXPECT_QUAT_EQ(bone->rotation, 0, 0, -0.5, 1.0/sqrt(3));
}
