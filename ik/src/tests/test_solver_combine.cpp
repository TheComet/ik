#include <gmock/gmock.h>
#include "ik/solver.h"
#include "ik/node.h"
#include "ik/cpputils.hpp"
#include "ik/quat.inl"

#define NAME solver_combine

using namespace testing;

TEST(NAME, two_target_one_bones)
{
    /*
     * n2   n1
     *   \  /
     *    n0
     */
    ik::Ref<ik_node> n0 = ik_node_create();
    ik::Ref<ik_node> n1 = ik_node_create_child(n0);
    ik::Ref<ik_node> n2 = ik_node_create_child(n0);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(n0, IK_ONE_BONE);
    ik::Ref<ik_effector> e1 = ik_node_create_effector(n1);
    ik::Ref<ik_effector> e2 = ik_node_create_effector(n2);
    ik::Ref<ik_solver> s = ik_solver_build(n0);

    ik_solver_solve(s);
}

TEST(NAME, two_target_one_bones_with_unaffected_children)
{
    /*
     * n2   n4
     * |     |
     * n1   n3
     *   \  /
     *    n0
     */
    ik::Ref<ik_node> n0 = ik_node_create();
    ik::Ref<ik_node> n1 = ik_node_create_child(n0);
    ik::Ref<ik_node> n2 = ik_node_create_child(n1);
    ik::Ref<ik_node> n3 = ik_node_create_child(n0);
    ik::Ref<ik_node> n4 = ik_node_create_child(n3);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(n0, IK_ONE_BONE);
    ik::Ref<ik_effector> e1 = ik_node_create_effector(n1);
    ik::Ref<ik_effector> e2 = ik_node_create_effector(n3);
    ik::Ref<ik_solver> s = ik_solver_build(n0);

    ik_vec3_set(n1->position.f, 0, 2, 0);
    ik_vec3_set(n2->position.f, 0, 0, 2);
    ik_vec3_set(n3->position.f, 0, -2, 0);
    ik_vec3_set(n4->position.f, 0, 0, 2);
    ik_vec3_set(e1->target_position.f, 0, 2, 0);
    ik_vec3_set(e2->target_position.f, 0, -2, 0);

    ik_quat_set_axis_angle(n1->rotation.f, 1, 0, 0, M_PI/2);
    ik_quat_set_axis_angle(n3->rotation.f, 1, 0, 0, M_PI/2);

    ik_solver_solve(s);
}

TEST(NAME, two_target_one_bones_with_third_unaffected_segment)
{
    /*
     * n1  n2 n3
     *   \ | /
     *    n0
     */
    ik::Ref<ik_node> n0 = ik_node_create();
    ik::Ref<ik_node> n1 = ik_node_create_child(n0);
    ik::Ref<ik_node> n2 = ik_node_create_child(n0);
    ik::Ref<ik_node> n3 = ik_node_create_child(n0);
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(n0, IK_ONE_BONE);
    ik::Ref<ik_effector> e1 = ik_node_create_effector(n1);
    ik::Ref<ik_effector> e2 = ik_node_create_effector(n3);
    ik::Ref<ik_solver> s = ik_solver_build(n0);

    ik_vec3_set(n1->position.f, 0, 2, 0);
    ik_vec3_set(n2->position.f, 0, 2, 2);
    ik_vec3_set(n3->position.f, 0, -2, 0);

    ik_vec3_set(e1->target_position.f, 0, 2, 0);
    ik_vec3_set(e2->target_position.f, 0, -2, 0);

    ik_solver_solve(s);
}
