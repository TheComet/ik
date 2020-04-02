#include "gmock/gmock.h"
#include "ik/node.h"
#include "ik/solver.h"
#include "ik/algorithm.h"
#include "ik/effector.h"
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

TEST_F(NAME, reach_target)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> mid = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(mid, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_TWO_BONE);

    base->position = ik_vec3(1, 2, 1);
    mid->position = ik_vec3(0, 2, 0);
    tip->position = ik_vec3(0, 2, 0);
    e->target_position = ik_vec3(2, -4, 0);

    // This should form the 3 nodes into an equilateral triangle
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_THAT(base->position.v.x, DoubleEq(1));
    EXPECT_THAT(base->position.v.y, DoubleEq(2));
    EXPECT_THAT(base->position.v.z, DoubleEq(1));

    // Mid position should remain the same in local space
    EXPECT_THAT(mid->position.v.x, DoubleEq(0));
    EXPECT_THAT(mid->position.v.y, DoubleEq(2));
    EXPECT_THAT(mid->position.v.z, DoubleEq(0));

    // Tip position should remain the same in local space
    EXPECT_THAT(tip->position.v.x, DoubleEq(0));
    EXPECT_THAT(tip->position.v.y, DoubleEq(2));
    EXPECT_THAT(tip->position.v.z, DoubleEq(0));

    // Base should have rotated about the Z axis by 60°
    EXPECT_THAT(base->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.z, DoubleEq(-0.5));
    EXPECT_THAT(base->rotation.q.w, DoubleEq(1.0/sqrt(3)));

    // Mid should have rotated about the Z axis by 60°
    EXPECT_THAT(mid->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(mid->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(mid->rotation.q.z, DoubleEq(-0.5));
    EXPECT_THAT(mid->rotation.q.w, DoubleEq(1.0/sqrt(3)));

    // Tip rotation should remain identical
    EXPECT_THAT(tip->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.z, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.w, DoubleEq(1.0));
}

TEST_F(NAME, reach_target_no_joint_rotations)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> mid = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(mid, ik_guid(2));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_TWO_BONE);

    base->position = ik_vec3(1, 2, 1);
    mid->position = ik_vec3(0, 2, 0);
    tip->position = ik_vec3(0, 2, 0);
    e->target_position = ik_vec3(2, -4, 0);
    a->features &= ~IK_ALGORITHM_JOINT_ROTATIONS;

    // This should form the 3 nodes into an equilateral triangle
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    // Base position should not change
    EXPECT_THAT(base->position.v.x, DoubleEq(1));
    EXPECT_THAT(base->position.v.y, DoubleEq(2));
    EXPECT_THAT(base->position.v.z, DoubleEq(1));

    // Mid position is at top of triangle
    EXPECT_THAT(mid->position.v.x, DoubleEq(1));
    EXPECT_THAT(mid->position.v.y, DoubleEq(sqrt(3)));
    EXPECT_THAT(mid->position.v.z, DoubleEq(0));

    // Tip position should remain the same in local space
    EXPECT_THAT(tip->position.v.x, DoubleEq(0));
    EXPECT_THAT(tip->position.v.y, DoubleEq(2));
    EXPECT_THAT(tip->position.v.z, DoubleEq(0));

    // Base should have rotated about the Z axis by 60°
    EXPECT_THAT(base->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.z, DoubleEq(-0.5));
    EXPECT_THAT(base->rotation.q.w, DoubleEq(1.0/sqrt(3)));

    // Mid should have rotated about the Z axis by 60°
    EXPECT_THAT(mid->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(mid->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(mid->rotation.q.z, DoubleEq(-0.5));
    EXPECT_THAT(mid->rotation.q.w, DoubleEq(1.0/sqrt(3)));

    // Tip rotation should remain identical
    EXPECT_THAT(tip->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.y, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.z, DoubleEq(0));
    EXPECT_THAT(tip->rotation.q.w, DoubleEq(1.0));
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

