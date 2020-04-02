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

TEST_F(NAME, rotate_90_degrees)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    base->position = ik_vec3(1, 2, 1);
    tip->position = ik_vec3(-2, 2, 0);
    e->target_position = ik_vec3(7, 3, 0);

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_THAT(base->position.v.x, DoubleEq(1));
    EXPECT_THAT(base->position.v.y, DoubleEq(2));
    EXPECT_THAT(base->position.v.z, DoubleEq(1));

    // Tip position should remain the same in local space
    EXPECT_THAT(tip->position.v.x, DoubleEq(-2));
    EXPECT_THAT(tip->position.v.y, DoubleEq(2));
    EXPECT_THAT(tip->position.v.z, DoubleEq(0));

    // Base should have rotated about the Z axis by 90°
    EXPECT_THAT(base->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.z, DoubleEq(-1.0/sqrt(2)));
    EXPECT_THAT(base->rotation.q.w, DoubleEq(1.0/sqrt(2)));

    // Tip rotation should remain identical
    EXPECT_THAT(tip->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.z, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.w, DoubleEq(1.0));
}

TEST_F(NAME, rotate_90_degrees_no_joint_rotations)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    base->position = ik_vec3(1, 2, 1);
    tip->position = ik_vec3(-2, 2, 0);
    e->target_position = ik_vec3(7, 3, 0);
    a->features &= ~IK_ALGORITHM_JOINT_ROTATIONS;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_THAT(base->position.v.x, DoubleEq(1));
    EXPECT_THAT(base->position.v.y, DoubleEq(2));
    EXPECT_THAT(base->position.v.z, DoubleEq(1));

    EXPECT_THAT(tip->position.v.x, DoubleEq(2));
    EXPECT_THAT(tip->position.v.y, DoubleEq(2));
    EXPECT_THAT(tip->position.v.z, DoubleEq(0));

    EXPECT_THAT(base->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.z, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.w, DoubleEq(1.0));

    EXPECT_THAT(tip->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.z, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.w, DoubleEq(1.0));
}

TEST_F(NAME, rotate_90_degrees_keep_effector_orientation)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    base->position = ik_vec3(1, 2, 1);
    tip->position = ik_vec3(-2, 2, 0);
    e->target_position = ik_vec3(7, 3, 0);
    e->features |= IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_THAT(base->position.v.x, DoubleEq(1));
    EXPECT_THAT(base->position.v.y, DoubleEq(2));
    EXPECT_THAT(base->position.v.z, DoubleEq(1));

    // Tip position should remain the same in local space
    EXPECT_THAT(tip->position.v.x, DoubleEq(-2));
    EXPECT_THAT(tip->position.v.y, DoubleEq(2));
    EXPECT_THAT(tip->position.v.z, DoubleEq(0));

    // Base should have rotated about the Z axis by 90°
    EXPECT_THAT(base->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.z, DoubleEq(-1.0/sqrt(2)));
    EXPECT_THAT(base->rotation.q.w, DoubleEq(1.0/sqrt(2)));

    // Tip rotation should be 90° about the Z axis in the opposite direction
    EXPECT_THAT(tip->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.z, DoubleEq(1.0/sqrt(2)));
    EXPECT_THAT(tip->rotation.q.w, DoubleEq(1.0/sqrt(2)));
}

TEST_F(NAME, rotate_90_degrees_keep_effector_orientation_no_join_rotations)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    base->position = ik_vec3(1, 2, 1);
    tip->position = ik_vec3(-2, 2, 0);
    e->target_position = ik_vec3(7, 3, 0);
    e->features |= IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION;
    a->features &= ~IK_ALGORITHM_JOINT_ROTATIONS;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_THAT(base->position.v.x, DoubleEq(1));
    EXPECT_THAT(base->position.v.y, DoubleEq(2));
    EXPECT_THAT(base->position.v.z, DoubleEq(1));

    EXPECT_THAT(tip->position.v.x, DoubleEq(2));
    EXPECT_THAT(tip->position.v.y, DoubleEq(2));
    EXPECT_THAT(tip->position.v.z, DoubleEq(0));

    EXPECT_THAT(base->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.z, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.w, DoubleEq(1.0));

    EXPECT_THAT(tip->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.z, DoubleEq(1.0/sqrt(2)));
    EXPECT_THAT(tip->rotation.q.w, DoubleEq(1.0/sqrt(2)));
}

TEST_F(NAME, already_pointing_at_target)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    base->position = ik_vec3(1, 2, 1);
    tip->position = ik_vec3(2, 2, 0);
    e->target_position = ik_vec3(3, 3, 0);

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    const double epsilon = 1e-7;
    EXPECT_THAT(base->position.v.x, DoubleNear(1, epsilon));
    EXPECT_THAT(base->position.v.y, DoubleNear(2, epsilon));
    EXPECT_THAT(base->position.v.z, DoubleNear(1, epsilon));

    EXPECT_THAT(tip->position.v.x, DoubleNear(2, epsilon));
    EXPECT_THAT(tip->position.v.y, DoubleNear(2, epsilon));
    EXPECT_THAT(tip->position.v.z, DoubleNear(0, epsilon));

    EXPECT_THAT(base->rotation.q.x, DoubleNear(0, epsilon));
    EXPECT_THAT(base->rotation.q.y, DoubleNear(0, epsilon));
    EXPECT_THAT(base->rotation.q.z, DoubleNear(0, epsilon));
    EXPECT_THAT(base->rotation.q.w, DoubleNear(1.0, epsilon));

    EXPECT_THAT(tip->rotation.q.x, DoubleNear(0, epsilon));
    EXPECT_THAT(tip->rotation.q.y, DoubleNear(0, epsilon));
    EXPECT_THAT(tip->rotation.q.z, DoubleNear(0, epsilon));
    EXPECT_THAT(tip->rotation.q.w, DoubleNear(1.0, epsilon));
}

TEST_F(NAME, rotate_90_degrees_with_constraint)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_constraint> c = ik_node_create_constraint(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    base->position = ik_vec3(1, 2, 1);
    tip->position = ik_vec3(0, 2, 0);
    e->target_position = ik_vec3(5, -2, 0);
    ik_constraint_set_type(c, IK_CONSTRAINT_CONE);
    c->data.cone.center = ik_vec3(0, 1, 0);
    c->data.cone.max_angle = M_PI / 3;  /* 60° */
    a->features |= IK_ALGORITHM_CONSTRAINTS;

    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_THAT(base->position.v.x, DoubleEq(1));
    EXPECT_THAT(base->position.v.y, DoubleEq(2));
    EXPECT_THAT(base->position.v.z, DoubleEq(1));

    EXPECT_THAT(tip->position.v.x, DoubleEq(0));
    EXPECT_THAT(tip->position.v.y, DoubleEq(2));
    EXPECT_THAT(tip->position.v.z, DoubleEq(0));

    // 60° rotation clockwise
    EXPECT_THAT(base->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.z, DoubleEq(-0.5));
    EXPECT_THAT(base->rotation.q.w, DoubleEq(1.0/sqrt(3)));

    EXPECT_THAT(tip->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.z, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.w, DoubleEq(1.0));
}
