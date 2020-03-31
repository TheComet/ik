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

TEST_F(NAME, rotate_single_bone_90_degrees)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);

    tip->position = ik_vec3(-2, 2, 0);
    e->target_position = ik_vec3(5, 5, 0);
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_THAT(base->position.v.x, DoubleEq(0));
    EXPECT_THAT(base->position.v.y, DoubleEq(0));
    EXPECT_THAT(base->position.v.z, DoubleEq(0));
    EXPECT_THAT(tip->position.v.x, DoubleEq(-2));
    EXPECT_THAT(tip->position.v.y, DoubleEq(2));
    EXPECT_THAT(tip->position.v.z, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.x, DoubleEq(0));
    EXPECT_THAT(base->rotation.q.x, DoubleEq(1.0/sqrt(2)));
    EXPECT_THAT(base->rotation.q.x, DoubleEq(1.0/sqrt(2)));
}

TEST_F(NAME, rotate_single_bone_90_degrees_no_joint_rotations)
{
    ik::Ref<ik_node> base = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(base, ik_guid(1));
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(base, IK_ONE_BONE);
    a->features &= ~IK_SOLVER_JOINT_ROTATIONS;

    tip->position = ik_vec3(-2, 2, 0);
    e->target_position = ik_vec3(5, 5, 0);
    ik::Ref<ik_solver> s = ik_solver_build(base);
    ik_solver_solve(s);

    EXPECT_THAT(base->position.v.x, DoubleEq(0));
    EXPECT_THAT(base->position.v.y, DoubleEq(0));
    EXPECT_THAT(base->position.v.z, DoubleEq(0));
    EXPECT_THAT(tip->position.v.x, DoubleEq(2));
    EXPECT_THAT(tip->position.v.y, DoubleEq(2));
    EXPECT_THAT(tip->position.v.z, DoubleEq(0));
}
