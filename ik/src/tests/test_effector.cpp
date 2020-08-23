#include "gmock/gmock.h"
#include "ik/effector.h"
#include "ik/node.h"
#include "ik/cpputils.hpp"

#define NAME effector

using namespace ::testing;

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

protected:
};

TEST_F(NAME, create_initializes_fields_properly)
{
    ik::Ref<ik_effector> eff = ik_effector_create();

    EXPECT_THAT(eff->target_position.v.x, DoubleEq(0));
    EXPECT_THAT(eff->target_position.v.y, DoubleEq(0));
    EXPECT_THAT(eff->target_position.v.z, DoubleEq(0));
    EXPECT_THAT(eff->target_rotation.q.x, DoubleEq(0));
    EXPECT_THAT(eff->target_rotation.q.y, DoubleEq(0));
    EXPECT_THAT(eff->target_rotation.q.z, DoubleEq(0));
    EXPECT_THAT(eff->target_rotation.q.w, DoubleEq(1));
    EXPECT_THAT(eff->weight, DoubleEq(1));
    EXPECT_THAT(eff->rotation_weight, DoubleEq(1));
    EXPECT_THAT(eff->rotation_decay, DoubleEq(0.25));
    EXPECT_THAT(eff->chain_length, Eq(0u));
    EXPECT_THAT(eff->features, Eq(0u));
}

#if 0
TEST_F(NAME, duplicate_copies_parameters_correctly)
{
    ik::Ref<ik_effector> eff = ik_effector_create();
    ik::Ref<ik_node> n = ik_node_create(0);

    eff->node = n; /* make node not null so we can test duplicated version */
    IKAPI.vec3.set(eff->target_position.f, 1, 2, 3);
    IKAPI.quat.set(eff->target_rotation.f, 4, 5, 6, 7);
    eff->weight = 0.4;
    eff->rotation_weight = 0.8;
    eff->rotation_decay = 0.12;
    eff->chain_length = 7;
    eff->flags = IK_EFFECTOR_WEIGHT_NLERP;

    ik::Ref<ik_effector> dup = IKAPI.effector.duplicate(eff);

    EXPECT_THAT(dup->node, IsNull());
    EXPECT_THAT(dup->target_position.x, DoubleEq(1));
    EXPECT_THAT(dup->target_position.y, DoubleEq(2));
    EXPECT_THAT(dup->target_position.z, DoubleEq(3));
    EXPECT_THAT(dup->target_rotation.x, DoubleEq(4));
    EXPECT_THAT(dup->target_rotation.y, DoubleEq(5));
    EXPECT_THAT(dup->target_rotation.z, DoubleEq(6));
    EXPECT_THAT(dup->target_rotation.w, DoubleEq(7));
    EXPECT_THAT(dup->weight, DoubleEq(0.4));
    EXPECT_THAT(dup->rotation_weight, DoubleEq(0.8));
    EXPECT_THAT(dup->rotation_decay, DoubleEq(0.12));
    EXPECT_THAT(dup->chain_length, Eq(7u));
    EXPECT_THAT(dup->flags, Eq(IK_EFFECTOR_WEIGHT_NLERP));

    IK_DECREF(eff);
    IK_DECREF(dup);
    IK_DECREF(n);
}
#endif

TEST_F(NAME, attach_detach_works)
{
    ik::Ref<ik_node> n = ik_node_create();
    ik::Ref<ik_effector> eff = ik_effector_create();

    ik_node_attach_effector(n, eff);
    EXPECT_THAT(n->effector, Eq(eff));
    EXPECT_THAT(eff->node, Eq(n));

    EXPECT_THAT(ik_node_detach_effector(n), Eq(eff));
    EXPECT_THAT(n->effector, IsNull());
    EXPECT_THAT(eff->node, IsNull());
}

TEST_F(NAME, reattach_removes_from_previous_node)
{
    ik::Ref<ik_node> n1 = ik_node_create();
    ik::Ref<ik_node> n2 = ik_node_create_child(n1);
    ik::Ref<ik_effector> eff = ik_effector_create();

    ik_node_attach_effector(n1, eff);
    EXPECT_THAT(n1->effector, Eq(eff));
    EXPECT_THAT(n2->effector, IsNull());
    EXPECT_THAT(eff->node, Eq(n1));

    ik_node_attach_effector(n2, eff);
    EXPECT_THAT(n1->effector, IsNull());
    EXPECT_THAT(n2->effector, Eq(eff));
    EXPECT_THAT(eff->node, Eq(n2));
}

TEST_F(NAME, attach_two_effectors_to_same_node)
{
    ik::Ref<ik_node> n = ik_node_create();
    ik::Ref<ik_effector> eff1 = ik_effector_create();
    ik::Ref<ik_effector> eff2 = ik_effector_create();

    ik_node_attach_effector(n, eff1);
    ik_node_attach_effector(n, eff2);
    EXPECT_THAT(n->effector, Eq(eff2));
    EXPECT_THAT(eff1->node, IsNull());
    EXPECT_THAT(eff2->node, Eq(n));
}

TEST_F(NAME, attach_already_attached_effector_again)
{
    ik::Ref<ik_node> n = ik_node_create();
    ik::Ref<ik_effector> eff = ik_effector_create();

    ik_node_attach_effector(n, eff);
    ik_node_attach_effector(n, eff);
    EXPECT_THAT(n->effector, Eq(eff));
    EXPECT_THAT(eff->node, Eq(n));
}
