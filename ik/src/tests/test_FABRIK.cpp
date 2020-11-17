#include "gmock/gmock.h"

#include "ik/effector.h"
#include "ik/bone.h"
#include "ik/solver.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"
#include "ik/cpputils.hpp"

#define NAME solver_fabrik

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

using namespace ::testing;

const double pi = std::atan(1.0) * 4;

class NAME : public Test
{
public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

protected:
};

static void buildTreeLongChainsRecurse(ik_solver* solver, ik_bone* parent, int depth)
{
    ik_bone* child1 = ik_bone_create(2);
    ik_bone* child2 = ik_bone_create(2);
    ik_bone* child3 = ik_bone_create(2);
    ik_bone* child4 = ik_bone_create(2);
    ik_bone* child5 = ik_bone_create(2);
    ik_bone* child6 = ik_bone_create(2);

    ik_bone_link(parent, child1);
    ik_bone_link(child1, child2);
    ik_bone_link(child2, child3);

    ik_bone_link(parent, child4);
    ik_bone_link(child4, child5);
    ik_bone_link(child5, child6);

    if(depth > 0)
    {
        buildTreeLongChainsRecurse(solver, child3, depth-1);
        buildTreeLongChainsRecurse(solver, child6, depth-1);
    }
    else
    {
        ik_bone_create_effector(child3);
        ik_bone_create_effector(child6);
    }
}
static void buildTreeLongChains(ik_solver* solver, int depth)
{
    ik_bone* root = ik_bone_create(2);
    buildTreeLongChainsRecurse(solver, root, depth);
}

static void buildTreeShortChainsRecurse(ik_solver* solver, ik_bone* parent, int depth)
{
    ik_bone* child1 = ik_bone_create(2);
    ik_bone* child2 = ik_bone_create(2);
    ik_bone_link(parent, child1);
    ik_bone_link(parent, child2);

    if(depth > 0)
    {
        buildTreeShortChainsRecurse(solver, child1, depth-1);
        buildTreeShortChainsRecurse(solver, child2, depth-1);
    }
    else
    {
        ik_bone_create_effector(child1);
        ik_bone_create_effector(child2);
    }
}
static void buildTreeShortChains(ik_solver* solver, int depth)
{
    ik_bone* root = ik_bone_create(2);
    buildTreeShortChainsRecurse(solver, root, depth);
}

TEST_F(NAME, two_bone_target_already_reached_should_iterate_0_times)
{
    ik::Ref<ik_bone> parent = ik_bone_create(2);
    ik::Ref<ik_bone> child = ik_bone_create_child(parent, 3);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(parent, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_bone_create_effector(child);

    ik_vec3_set(e->target_position.f, 0, 0, 5);

    a->tolerance = 0.01;
    ik::Ref<ik_solver> s = ik_solver_build(parent);
    EXPECT_THAT(ik_solver_solve(s), Eq(0));

    EXPECT_QUAT_EQ(parent->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_EQ(child->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, two_bone_target_initial_angles)
{
    ik::Ref<ik_bone> parent = ik_bone_create(2);
    ik::Ref<ik_bone> child = ik_bone_create_child(parent, 2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(parent, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_bone_create_effector(child);

    ik_vec3_set(e->target_position.f, 0, 0, 4.5); // target is slightly out of reach

    // set some random angles
    ik_quat_set(parent->rotation.f, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2));
    ik_quat_set(child->rotation.f, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2));

    a->max_iterations = 20;
    ik::Ref<ik_solver> s = ik_solver_build(parent);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(parent->rotation.f);
    ik_quat_ensure_positive_sign(child->rotation.f);
    EXPECT_QUAT_NEAR(parent->rotation, 0, 0, 0, 1, 1e-4);
    EXPECT_QUAT_NEAR(child->rotation, 0, 0, 0, 1, 1e-4);
}

TEST_F(NAME, one_bone_target)
{
    ik::Ref<ik_bone> bone = ik_bone_create(2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(bone, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_bone_create_effector(bone);

    ik_vec3_set(e->target_position.f, 0, 2.5, 0); // target is slightly out of reach

    a->max_iterations = 1;
    ik::Ref<ik_solver> s = ik_solver_build(bone);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(bone->rotation.f);
    EXPECT_QUAT_NEAR(bone->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-4);
    EXPECT_VEC3_EQ(bone->position, 0, 0, 0);
}

TEST_F(NAME, one_bone_target_with_root_offset)
{
    ik::Ref<ik_bone> bone = ik_bone_create(2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(bone, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_bone_create_effector(bone);

    ik_vec3_set(bone->position.f, 3, 3, 3);
    ik_vec3_set(e->target_position.f, 3, 5.5, 3); // target is slightly out of reach

    a->max_iterations = 1;
    ik::Ref<ik_solver> s = ik_solver_build(bone);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(bone->rotation.f);
    EXPECT_QUAT_NEAR(bone->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-4);
    EXPECT_VEC3_EQ(bone->position, 3, 3, 3);
}

TEST_F(NAME, one_bone_target_with_leaf_child)
{
    ik::Ref<ik_bone> bone = ik_bone_create(2);
    ik::Ref<ik_bone> leaf = ik_bone_create_child(bone, 2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(bone, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_bone_create_effector(bone);

    ik_vec3_set(e->target_position.f, 0, 2.5, 0); // target is slightly out of reach

    a->max_iterations = 20;
    ik::Ref<ik_solver> s = ik_solver_build(bone);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(bone->rotation.f);
    EXPECT_QUAT_NEAR(bone->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-4);
    EXPECT_QUAT_EQ(leaf->rotation, 0, 0, 0, 1);
    EXPECT_VEC3_EQ(bone->position, 0, 0, 0);
    EXPECT_VEC3_EQ(leaf->position, 0, 0, 0);
}

TEST_F(NAME, two_bone_target_right_angle)
{
    ik::Ref<ik_bone> parent = ik_bone_create(2);
    ik::Ref<ik_bone> child = ik_bone_create_child(parent, 2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(parent, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_bone_create_effector(child);

    /*ik_quat_set(mid->rotation.f, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2));*/
    ik_vec3_set(e->target_position.f, 0, 2, 2);

    a->max_iterations = 1;
    ik::Ref<ik_solver> s = ik_solver_build(parent);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(parent->rotation.f);
    EXPECT_QUAT_EQ(parent->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_NEAR(child->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-4);
}

TEST_F(NAME, two_targets_already_reached)
{
    ik::Ref<ik_bone> root = ik_bone_create(2);
    ik::Ref<ik_bone> center = ik_bone_create_child(root, 2);
    ik::Ref<ik_bone> midl = ik_bone_create_child(center, 2*sqrt(2));
    ik::Ref<ik_bone> tipl = ik_bone_create_child(midl, 2);
    ik::Ref<ik_bone> midr = ik_bone_create_child(center, 2*sqrt(2));
    ik::Ref<ik_bone> tipr = ik_bone_create_child(midr, 2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> el = ik_bone_create_effector(tipl);
    ik::Ref<ik_effector> er = ik_bone_create_effector(tipr);

    ik_vec3_set(el->target_position.f, 0, -4, 8);
    ik_vec3_set(er->target_position.f, 0, 4, 8);

    ik_quat_set_axis_angle(midl->rotation.f, 1, 0, 0, pi/4);
    ik_quat_set_axis_angle(midr->rotation.f, 1, 0, 0, -pi/4);

    a->max_iterations = 20;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(center->rotation.f);
    ik_quat_ensure_positive_sign(midl->rotation.f);
    ik_quat_ensure_positive_sign(tipl->rotation.f);
    ik_quat_ensure_positive_sign(midr->rotation.f);
    ik_quat_ensure_positive_sign(tipr->rotation.f);

    EXPECT_QUAT_NEAR(root->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(center->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(midl->rotation, sin(pi/8), 0, 0, cos(pi/8), 1e-7);
    EXPECT_QUAT_NEAR(tipl->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(midr->rotation, -sin(pi/8), 0, 0, cos(pi/8), 1e-7);
    EXPECT_QUAT_NEAR(tipr->rotation, 0, 0, 0, 1, 1e-7);
}

TEST_F(NAME, two_targets_with_offset)
{
    ik::Ref<ik_bone> root = ik_bone_create(2);
    ik::Ref<ik_bone> tipl = ik_bone_create_child(root, 2);
    ik::Ref<ik_bone> tipr = ik_bone_create_child(root, 2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> el = ik_bone_create_effector(tipl);
    ik::Ref<ik_effector> er = ik_bone_create_effector(tipr);

    ik_vec3_set(tipr->position.f, 0, 2, 0);

    ik_vec3_set(el->target_position.f, 0, -2, 2);
    ik_vec3_set(er->target_position.f, 0, 4, 2);

    a->max_iterations = 1;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    EXPECT_QUAT_NEAR(root->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(tipl->rotation, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
    EXPECT_QUAT_NEAR(tipr->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
}

TEST_F(NAME, two_targets_two_bones)
{
    ik::Ref<ik_bone> root = ik_bone_create(2);
    ik::Ref<ik_bone> mid1 = ik_bone_create_child(root, 2);
    ik::Ref<ik_bone> midl = ik_bone_create_child(mid1, 2);
    ik::Ref<ik_bone> tipl = ik_bone_create_child(midl, 2);
    ik::Ref<ik_bone> midr = ik_bone_create_child(mid1, 2);
    ik::Ref<ik_bone> tipr = ik_bone_create_child(midr, 2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> el = ik_bone_create_effector(tipl);
    ik::Ref<ik_effector> er = ik_bone_create_effector(tipr);

    ik_quat_set_axis_angle(midl->rotation.f, 1, 0, 0, pi/2);
    ik_quat_set_axis_angle(midr->rotation.f, 1, 0, 0, -pi/2);

    ik_vec3_set(el->target_position.f, 0, -5, 4);
    ik_vec3_set(er->target_position.f, 0, 5, 4);

    a->max_iterations = 20;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(mid1->rotation.f);
    ik_quat_ensure_positive_sign(midl->rotation.f);
    ik_quat_ensure_positive_sign(tipl->rotation.f);
    ik_quat_ensure_positive_sign(midr->rotation.f);
    ik_quat_ensure_positive_sign(tipr->rotation.f);

    EXPECT_QUAT_NEAR(root->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(mid1->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(midl->rotation, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
    EXPECT_QUAT_NEAR(midr->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
    EXPECT_QUAT_NEAR(tipl->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(tipr->rotation, 0, 0, 0, 1, 1e-7);
}

TEST_F(NAME, two_targets_two_bones_with_offset)
{
    ik::Ref<ik_bone> root = ik_bone_create(2);
    ik::Ref<ik_bone> mid1 = ik_bone_create_child(root, 2);
    ik::Ref<ik_bone> midl = ik_bone_create_child(mid1, 2);
    ik::Ref<ik_bone> tipl = ik_bone_create_child(midl, 2);
    ik::Ref<ik_bone> midr = ik_bone_create_child(mid1, 2);
    ik::Ref<ik_bone> tipr = ik_bone_create_child(midr, 2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> el = ik_bone_create_effector(tipl);
    ik::Ref<ik_effector> er = ik_bone_create_effector(tipr);

    ik_vec3_set(midr->position.f, 0, 2, 0);
    ik_vec3_set(tipr->position.f, 0, 0, 2);

    ik_quat_set_axis_angle(midl->rotation.f, 1, 0, 0, pi/2);
    ik_quat_set_axis_angle(midr->rotation.f, 1, 0, 0, -pi/2);

    ik_vec3_set(el->target_position.f, 0, -4, 4);
    ik_vec3_set(er->target_position.f, 0, 8, 4);

    a->max_iterations = 20;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    EXPECT_QUAT_NEAR(root->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(mid1->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(midl->rotation, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
    EXPECT_QUAT_NEAR(midr->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
    EXPECT_QUAT_NEAR(tipl->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(tipr->rotation, 0, 0, 0, 1, 1e-7);
}

TEST_F(NAME, two_targets_two_bones_with_one_target_slightly_out_of_reach)
{
    ik::Ref<ik_bone> root = ik_bone_create(2);
    ik::Ref<ik_bone> mid1 = ik_bone_create_child(root, 2);
    ik::Ref<ik_bone> midl = ik_bone_create_child(mid1, 2);
    ik::Ref<ik_bone> tipl = ik_bone_create_child(midl, 2);
    ik::Ref<ik_bone> midr = ik_bone_create_child(mid1, 2);
    ik::Ref<ik_bone> tipr = ik_bone_create_child(midr, 2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> el = ik_bone_create_effector(tipl);
    ik::Ref<ik_effector> er = ik_bone_create_effector(tipr);

    ik_quat_set_axis_angle(midl->rotation.f, 1, 0, 0, pi/2);
    ik_quat_set_axis_angle(midr->rotation.f, 1, 0, 0, -pi/2);

    // This caused a bug where the two base bones of each child chain would
    // rotate too far (midl and midr). We expect the rotations to be almost
    // identical to the previous test
    ik_vec3_set(el->target_position.f, 0, -4, 4);
    ik_vec3_set(er->target_position.f, 0, 4.01, 4);

    // Bug is more noticable with more iterations for some reason
    a->max_iterations = 50;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(mid1->rotation.f);
    ik_quat_ensure_positive_sign(midl->rotation.f);
    ik_quat_ensure_positive_sign(tipl->rotation.f);
    ik_quat_ensure_positive_sign(midr->rotation.f);
    ik_quat_ensure_positive_sign(tipr->rotation.f);

    const ikreal tolerance = 1e-2;
    EXPECT_QUAT_NEAR(root->rotation, 0, 0, 0, 1, tolerance);
    EXPECT_QUAT_NEAR(mid1->rotation, 0, 0, 0, 1, tolerance);
    EXPECT_QUAT_NEAR(midl->rotation, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2), tolerance);
    EXPECT_QUAT_NEAR(midr->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), tolerance);
    EXPECT_QUAT_NEAR(tipl->rotation, 0, 0, 0, 1, tolerance);
    EXPECT_QUAT_NEAR(tipr->rotation, 0, 0, 0, 1, tolerance);
}

TEST_F(NAME, two_targets_two_bones_one_target_out_of_reach_other_in_reach)
{
    ik::Ref<ik_bone> root = ik_bone_create(sqrt(2));
    ik::Ref<ik_bone> mid1 = ik_bone_create_child(root, sqrt(2));
    ik::Ref<ik_bone> midl = ik_bone_create_child(mid1, sqrt(2));
    ik::Ref<ik_bone> tipl = ik_bone_create_child(midl, sqrt(2));
    ik::Ref<ik_bone> midr = ik_bone_create_child(mid1, sqrt(2));
    ik::Ref<ik_bone> tipr = ik_bone_create_child(midr, sqrt(2));
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> el = ik_bone_create_effector(tipl);
    ik::Ref<ik_effector> er = ik_bone_create_effector(tipr);

    ik_vec3_set(el->target_position.f, 0, -5, 5);  // Stretch left arm (slightly out of reach)
    ik_vec3_set(er->target_position.f, 0, -2, 2);  // Relax right arm (easily in reach)

    // Bug is more noticable with more iterations for some reason
    a->max_iterations = 50;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(mid1->rotation.f);
    ik_quat_ensure_positive_sign(midl->rotation.f);
    ik_quat_ensure_positive_sign(tipl->rotation.f);

    const ikreal tolerance = 1e-4;
    EXPECT_QUAT_NEAR(root->rotation, sin(pi/8), 0, 0, cos(pi/8), tolerance);
    EXPECT_QUAT_NEAR(mid1->rotation, 0, 0, 0, 1, tolerance);
    EXPECT_QUAT_NEAR(midl->rotation, 0, 0, 0, 1, tolerance);
    EXPECT_QUAT_NEAR(tipl->rotation, 0, 0, 0, 1, tolerance);
}

TEST_F(NAME, two_bone_stiff_constraint)
{
    ik::Ref<ik_bone> parent = ik_bone_create(2);
    ik::Ref<ik_bone> child = ik_bone_create_child(parent, 2);
    ik::Ref<ik_algorithm> a = ik_bone_create_algorithm(parent, IK_FABRIK);
    ik::Ref<ik_constraint> c = ik_bone_create_constraint(child);
    ik::Ref<ik_effector> e = ik_bone_create_effector(child);

    ik_vec3_set(e->target_position.f, 0, sqrt(4*4 - 2*2), 2);
    ik_constraint_set_stiff(c, 0, 0, 0, 1);

    a->max_iterations = 1;
    a->features |= IK_ALGORITHM_CONSTRAINTS;

    ik::Ref<ik_solver> s = ik_solver_build(parent);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(parent->rotation.f);
    ik_quat_ensure_positive_sign(child->rotation.f);

    EXPECT_QUAT_EQ(parent->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_NEAR(child->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
}

TEST_F(NAME, embedded_effector)
{
    ik::Ref<ik_bone> root = ik_bone_create(2);
    ik::Ref<ik_bone> mid = ik_bone_create_child(root, 2);
    ik::Ref<ik_bone> tip = ik_bone_create_child(mid, 2);
    ik::Ref<ik_algorithm> a1 = ik_bone_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> e1 = ik_bone_create_effector(root);
    ik::Ref<ik_algorithm> a2 = ik_bone_create_algorithm(mid, IK_FABRIK);
    ik::Ref<ik_effector> e2 = ik_bone_create_effector(tip);

    ik_vec3_set(e1->target_position.f, 0, 2.5, 0);
    ik_vec3_set(e2->target_position.f, 0, 2, 4.5);

    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(mid->rotation.f);

    EXPECT_QUAT_NEAR(root->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-4);
    EXPECT_QUAT_NEAR(mid->rotation, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-4);
}
