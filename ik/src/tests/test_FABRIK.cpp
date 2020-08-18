#include "gmock/gmock.h"

#include "ik/effector.h"
#include "ik/node.h"
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

static void buildTreeLongChainsRecurse(ik_solver* solver, ik_node* parent, int depth, int* guid)
{
    ik_node* child1 = ik_node_create(ik_guid((*guid)++));
    ik_node* child2 = ik_node_create(ik_guid((*guid)++));
    ik_node* child3 = ik_node_create(ik_guid((*guid)++));
    ik_node* child4 = ik_node_create(ik_guid((*guid)++));
    ik_node* child5 = ik_node_create(ik_guid((*guid)++));
    ik_node* child6 = ik_node_create(ik_guid((*guid)++));

    ik_node_link(parent, child1);
    ik_node_link(child1, child2);
    ik_node_link(child2, child3);

    ik_node_link(parent, child4);
    ik_node_link(child4, child5);
    ik_node_link(child5, child6);

    if(depth > 0)
    {
        buildTreeLongChainsRecurse(solver, child3, depth-1, guid);
        buildTreeLongChainsRecurse(solver, child6, depth-1, guid);
    }
    else
    {
        ik_node_create_effector(child3);
        ik_node_create_effector(child6);
    }
}
static void buildTreeLongChains(ik_solver* solver, int depth)
{
    int guid = 0;
    ik_node* root = ik_node_create(ik_guid(guid++));
    buildTreeLongChainsRecurse(solver, root, depth, &guid);
}

static void buildTreeShortChainsRecurse(ik_solver* solver, ik_node* parent, int depth, int* guid)
{
    ik_node* child1 = ik_node_create(ik_guid((*guid)++));
    ik_node* child2 = ik_node_create(ik_guid((*guid)++));
    ik_node_link(parent, child1);
    ik_node_link(parent, child2);

    if(depth > 0)
    {
        buildTreeShortChainsRecurse(solver, child1, depth-1, guid);
        buildTreeShortChainsRecurse(solver, child2, depth-1, guid);
    }
    else
    {
        ik_node_create_effector(child1);
        ik_node_create_effector(child2);
    }
}
static void buildTreeShortChains(ik_solver* solver, int depth)
{
    int guid = 0;
    ik_node* root = ik_node_create(ik_guid(guid++));
    buildTreeShortChainsRecurse(solver, root, depth, &guid);
}

TEST_F(NAME, two_bone_target_already_reached_should_iterate_0_times)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> mid = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(mid, ik_guid(2));
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);

    ik_vec3_set(root->position.f, 0, 0, 0);
    ik_vec3_set(mid->position.f, 0, 0, 2);
    ik_vec3_set(tip->position.f, 0, 0, 3);
    ik_vec3_set(e->target_position.f, 0, 0, 5);

    a->tolerance = 0.01;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    EXPECT_THAT(ik_solver_solve(s), Eq(0));

    EXPECT_QUAT_EQ(root->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_EQ(mid->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, two_bone_target_initial_angles)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> mid = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(mid, ik_guid(2));
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);

    ik_vec3_set(root->position.f, 0, 0, 0);
    ik_vec3_set(mid->position.f, 0, 0, 2);
    ik_vec3_set(tip->position.f, 0, 0, 2);
    ik_vec3_set(e->target_position.f, 0, 0, 4.5); // target is slightly out of reach

    // set some random angles
    ik_quat_set(root->rotation.f, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2));
    ik_quat_set(mid->rotation.f, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2));

    a->max_iterations = 20;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(mid->rotation.f);
    EXPECT_QUAT_NEAR(root->rotation, 0, 0, 0, 1, 1e-4);
    EXPECT_QUAT_NEAR(mid->rotation, 0, 0, 0, 1, 1e-4);
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, one_bone_target)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);

    ik_vec3_set(root->position.f, 0, 0, 0);
    ik_vec3_set(tip->position.f, 0, 0, 2);
    ik_vec3_set(e->target_position.f, 0, 2.5, 0); // target is slightly out of reach

    a->max_iterations = 1;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    EXPECT_QUAT_NEAR(root->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-4);
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, one_bone_target_with_root_offset)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);

    ik_vec3_set(root->position.f, 3, 3, 3);
    ik_vec3_set(tip->position.f, 0, 0, 2);
    ik_vec3_set(e->target_position.f, 3, 5.5, 3); // target is slightly out of reach

    a->max_iterations = 1;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    EXPECT_QUAT_NEAR(root->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-4);
    EXPECT_QUAT_EQ(tip->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, one_bone_target_with_leaf_child)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> tip = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> leaf = ik_node_create_child(tip, ik_guid(2));
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);

    ik_vec3_set(root->position.f, 0, 0, 0);
    ik_vec3_set(tip->position.f, 0, 0, 2);
    ik_vec3_set(leaf->position.f, 0, 0, 2);
    ik_vec3_set(e->target_position.f, 0, 2.5, 0); // target is slightly out of reach
    ik_quat_set(tip->rotation.f, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2));

    a->max_iterations = 20;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    EXPECT_QUAT_NEAR(root->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-4);
    EXPECT_QUAT_NEAR(tip->rotation, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
    EXPECT_QUAT_EQ(leaf->rotation, 0, 0, 0, 1);
}

TEST_F(NAME, two_targets_already_reached)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> mid = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> center = ik_node_create_child(mid, ik_guid(2));
    ik::Ref<ik_node> midl = ik_node_create_child(center, ik_guid(3));
    ik::Ref<ik_node> tipl = ik_node_create_child(midl, ik_guid(4));
    ik::Ref<ik_node> midr = ik_node_create_child(center, ik_guid(5));
    ik::Ref<ik_node> tipr = ik_node_create_child(midr, ik_guid(6));
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> el = ik_node_create_effector(tipl);
    ik::Ref<ik_effector> er = ik_node_create_effector(tipr);

    ik_vec3_set(root->position.f, 0, 0, 0);
    ik_vec3_set(mid->position.f, 0, 0, 2);
    ik_vec3_set(center->position.f, 0, 0, 2);
    ik_vec3_set(midl->position.f, 0, -2, 2);
    ik_vec3_set(tipl->position.f, 0, 0, 2);
    ik_vec3_set(midr->position.f, 0, 2, 2);
    ik_vec3_set(tipr->position.f, 0, 0, 2);

    ik_vec3_set(el->target_position.f, 0, -2, 8);
    ik_vec3_set(er->target_position.f, 0, 2, 8);

    a->max_iterations = 20;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(mid->rotation.f);
    ik_quat_ensure_positive_sign(center->rotation.f);
    ik_quat_ensure_positive_sign(midl->rotation.f);
    ik_quat_ensure_positive_sign(tipl->rotation.f);
    ik_quat_ensure_positive_sign(midr->rotation.f);
    ik_quat_ensure_positive_sign(tipr->rotation.f);

    EXPECT_QUAT_NEAR(root->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(mid->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(center->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(midl->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(tipl->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(midr->rotation, 0, 0, 0, 1, 1e-7);
    EXPECT_QUAT_NEAR(tipr->rotation, 0, 0, 0, 1, 1e-7);
}

TEST_F(NAME, two_targets)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> mid = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> center = ik_node_create_child(mid, ik_guid(2));
    ik::Ref<ik_node> midl = ik_node_create_child(center, ik_guid(3));
    ik::Ref<ik_node> tipl = ik_node_create_child(midl, ik_guid(4));
    ik::Ref<ik_node> midr = ik_node_create_child(center, ik_guid(5));
    ik::Ref<ik_node> tipr = ik_node_create_child(midr, ik_guid(6));
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> el = ik_node_create_effector(tipl);
    ik::Ref<ik_effector> er = ik_node_create_effector(tipr);

    ik_vec3_set(root->position.f, 0, 0, 0);
    ik_vec3_set(mid->position.f, 0, 0, 2);
    ik_vec3_set(center->position.f, 0, 0, 2);
    ik_vec3_set(midl->position.f, 0, -2, 2);
    ik_vec3_set(tipl->position.f, 0, 0, 2);
    ik_vec3_set(midr->position.f, 0, 2, 2);
    ik_vec3_set(tipr->position.f, 0, 0, 2);

    ik_vec3_set(el->target_position.f, 0, 2, 6);
    ik_vec3_set(er->target_position.f, 0, 3, 0);

    a->max_iterations = 20;
    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(mid->rotation.f);
    ik_quat_ensure_positive_sign(center->rotation.f);
    ik_quat_ensure_positive_sign(midl->rotation.f);
    ik_quat_ensure_positive_sign(tipl->rotation.f);
    ik_quat_ensure_positive_sign(midr->rotation.f);
    ik_quat_ensure_positive_sign(tipr->rotation.f);
}

TEST_F(NAME, two_bone_stiff_constraint)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> mid = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(mid, ik_guid(2));
    ik::Ref<ik_algorithm> a = ik_node_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_constraint> c = ik_node_create_constraint(mid);
    ik::Ref<ik_effector> e = ik_node_create_effector(tip);

    ik_vec3_set(mid->position.f, 0, 0, 2);
    ik_vec3_set(tip->position.f, 0, 0, 2);
    ik_vec3_set(e->target_position.f, 0, sqrt(4*4 - 2*2), 2);
    ik_constraint_set_stiff(c, 0, 0, 0, 1);

    a->max_iterations = 1;
    a->features |= IK_ALGORITHM_CONSTRAINTS;

    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(mid->rotation.f);

    EXPECT_QUAT_EQ(root->rotation, 0, 0, 0, 1);
    EXPECT_QUAT_NEAR(mid->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
}

TEST_F(NAME, embedded_effector)
{
    ik::Ref<ik_node> root = ik_node_create(ik_guid(0));
    ik::Ref<ik_node> mid = ik_node_create_child(root, ik_guid(1));
    ik::Ref<ik_node> tip = ik_node_create_child(mid, ik_guid(2));
    ik::Ref<ik_algorithm> a1 = ik_node_create_algorithm(root, IK_FABRIK);
    ik::Ref<ik_effector> e1 = ik_node_create_effector(mid);
    ik::Ref<ik_algorithm> a2 = ik_node_create_algorithm(mid, IK_FABRIK);
    ik::Ref<ik_effector> e2 = ik_node_create_effector(tip);

    ik_vec3_set(mid->position.f, 0, 0, 2);
    ik_vec3_set(tip->position.f, 0, 0, 2);
    ik_vec3_set(e1->target_position.f, 0, 2, 0);
    ik_vec3_set(e2->target_position.f, 0, 2, 2);

    ik::Ref<ik_solver> s = ik_solver_build(root);
    ik_solver_solve(s);

    ik_quat_ensure_positive_sign(root->rotation.f);
    ik_quat_ensure_positive_sign(mid->rotation.f);

    EXPECT_QUAT_NEAR(root->rotation, -1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
    EXPECT_QUAT_NEAR(mid->rotation, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2), 1e-7);
}
