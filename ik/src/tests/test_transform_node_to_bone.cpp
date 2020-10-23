#include "gmock/gmock.h"

#include "ik/bone.h"
#include "ik/chain_tree.h"
#include "ik/subtree.h"
#include "ik/transform.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"
#include "ik/cpputils.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

#define NAME transform_segmental

using namespace ::testing;

class Chain
{
public:
    Chain(ik_subtree* subtree)
    {
        chain_tree_init(&chain_);
        chain_tree_build(&chain_, subtree);
    }
    ~Chain()
    {
        chain_tree_deinit(&chain_);
    }

    ik_chain* get() { return &chain_; }

private:
    ik_chain chain_;
};

class Subtree
{
public:
    Subtree()
    {
        subtree_init(&st_);
    }

    ~Subtree()
    {
        subtree_deinit(&st_);
    }

    ik_subtree* get() { return &st_; }

private:
    ik_subtree st_;
};

class NAME : public Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    Chain MakeOneBone(ik::Ref<ik_bone>* base, ik::Ref<ik_bone>* tip)
    {
        *base = ik_bone_create();
        *tip  = ik_bone_create_child(*base);

        ik_vec3_set((*tip)->position.f, 0, 0, 2);

        Subtree st;
        subtree_set_root(st.get(), *base);
        subtree_add_leaf(st.get(), *tip);
        return Chain(st.get());
    }

    Chain MakeTwoBone(ik::Ref<ik_bone>* base, ik::Ref<ik_bone>* mid, ik::Ref<ik_bone>* tip)
    {
        *base = ik_bone_create();
        *mid  = ik_bone_create_child(*base);
        *tip  = ik_bone_create_child(*mid);

        ik_vec3_set((*mid)->position.f, 0, 0, 2);
        ik_vec3_set((*tip)->position.f, 0, 0, 2);

        Subtree st;
        subtree_set_root(st.get(), *base);
        subtree_add_leaf(st.get(), *tip);
        return Chain(st.get());
    }

    Chain MakeOneBoneSplit(ik::Ref<ik_bone>* base, ik::Ref<ik_bone>* mid, ik::Ref<ik_bone>* tipl, ik::Ref<ik_bone>* tipr)
    {
        *base = ik_bone_create();
        *mid  = ik_bone_create_child(*base);
        *tipl = ik_bone_create_child(*mid);
        *tipr = ik_bone_create_child(*mid);

        ik_vec3_set((*mid)->position.f, 0, 0, 2);
        ik_vec3_set((*tipl)->position.f, 0, 0, 2);
        ik_vec3_set((*tipr)->position.f, 0, 0, 2);

        Subtree st;
        subtree_set_root(st.get(), *base);
        subtree_add_leaf(st.get(), *tipl);
        subtree_add_leaf(st.get(), *tipr);
        return Chain(st.get());
    }

    float RandomFloat()
    {
        return ((float)rand()/(float)(RAND_MAX));
    }

    ik_quat RandomRotation()
    {
        ik_quat q;
        q.q.x = RandomFloat() * 2 - 1;
        q.q.y = RandomFloat() * 2 - 1;
        q.q.z = RandomFloat() * 2 - 1;
        q.q.w = RandomFloat() * 2 - 1;
        ik_quat_normalize(q.f);
        return q;
    }

protected:
};

#define EXPECT_QUAT_EQ(q1, q2) do {                                           \
        EXPECT_THAT(q1.q.x, DoubleEq(q2.q.x));                                \
        EXPECT_THAT(q1.q.y, DoubleEq(q2.q.y));                                \
        EXPECT_THAT(q1.q.z, DoubleEq(q2.q.z));                                \
        EXPECT_THAT(q1.q.w, DoubleEq(q2.q.w));                                \
    } while(0)

#define EXPECT_QUAT_NEAR(q1, q2, epsilon) do {                                \
        EXPECT_THAT(q1.q.x, DoubleNear(q2.q.x, epsilon));                     \
        EXPECT_THAT(q1.q.y, DoubleNear(q2.q.y, epsilon));                     \
        EXPECT_THAT(q1.q.z, DoubleNear(q2.q.z, epsilon));                     \
        EXPECT_THAT(q1.q.w, DoubleNear(q2.q.w, epsilon));                     \
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

TEST_F(NAME, one_bone)
{
    ik::Ref<ik_bone> base, tip;
    Chain c = MakeOneBone(&base, &tip);
    ik_quat ir[1];

    ik_quat q0 = RandomRotation();
    ik_quat q1 = RandomRotation();
    base->rotation = q0;
    tip->rotation = q1;

    //ik_transform_chain_to_segmental_representation(c.get(), ir, 1);

    EXPECT_QUAT_EQ(base->rotation, q0);
    EXPECT_QUAT_EQ(tip->rotation, q0);
    EXPECT_QUAT_EQ(ir[0], q1);
}

TEST_F(NAME, two_bone)
{
    ik::Ref<ik_bone> base, mid, tip;
    Chain c = MakeTwoBone(&base, &mid, &tip);
    ik_quat ir[1];

    ik_quat q0 = RandomRotation();
    ik_quat q1 = RandomRotation();
    ik_quat q2 = RandomRotation();
    base->rotation = q0;
    mid->rotation = q1;
    tip->rotation = q2;

    //ik_transform_chain_to_segmental_representation(c.get(), ir, 1);

    EXPECT_QUAT_EQ(base->rotation, q0);
    EXPECT_QUAT_EQ(mid->rotation, q0);
    EXPECT_QUAT_EQ(tip->rotation, q1);
    EXPECT_QUAT_EQ(ir[0], q2);
}

TEST_F(NAME, one_bone_split)
{
    ik::Ref<ik_bone> base, mid, tipl, tipr;
    Chain c = MakeOneBoneSplit(&base, &mid, &tipl, &tipr);
    ik_quat ir[3];

    ik_quat q0 = RandomRotation();
    ik_quat q1; ik_quat_set(q1.f, 0, 0, 0, 1);
    ik_quat q2 = RandomRotation();
    ik_quat q3 = RandomRotation();
    base->rotation = q0;
    mid->rotation = q1;
    tipl->rotation = q2;
    tipr->rotation = q3;

    ik_vec3_set(tipr->position.f, 0, 1, 1);
    ik_vec3_set(tipl->position.f, 0, -1, 1);

    //ik_transform_chain_to_segmental_representation(c.get(), ir, 3);

    ik_quat ql; ik_quat_set_axis_angle(ql.f, 1, 0, 0, M_PI/4);
    ik_quat qr; ik_quat_set_axis_angle(qr.f, 1, 0, 0, -M_PI/4);

    EXPECT_QUAT_EQ(base->rotation, q0);
    EXPECT_QUAT_EQ(mid->rotation, q0);
    EXPECT_QUAT_NEAR(tipl->rotation, ql, 1e-7);
    EXPECT_QUAT_NEAR(tipr->rotation, qr, 1e-7);
    EXPECT_VEC3_EQ(tipr->position, 0, 0, sqrt(2));
    EXPECT_VEC3_EQ(tipl->position, 0, 0, sqrt(2));

    //ik_transform_chain_to_nodal_representation(c.get(), ir, 3);

    EXPECT_QUAT_EQ(base->rotation, q0);
    EXPECT_QUAT_EQ(mid->rotation, q1);
    EXPECT_QUAT_NEAR(tipl->rotation, q2, 1e-7);
    EXPECT_QUAT_NEAR(tipr->rotation, q3, 1e-7);
    EXPECT_VEC3_EQ(tipr->position, 0, 1, 1);
    EXPECT_VEC3_EQ(tipl->position, 0, -1, 1);
}
