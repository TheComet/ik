#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME effector

using namespace ::testing;

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
        solver_ = IKAPI.solver.create(IKAPI.solver.FABRIK);
    }

    virtual void TearDown() override
    {
        IKAPI.solver.destroy(solver_);
    }

protected:
    ik_solver_t* solver_;
};

TEST_F(NAME, create_initializes_fields_properly)
{
    ik_effector_t* eff = IKAPI.effector.create();

    EXPECT_THAT(eff->node, IsNull());
    EXPECT_THAT(eff->target_position.x, DoubleEq(0));
    EXPECT_THAT(eff->target_position.y, DoubleEq(0));
    EXPECT_THAT(eff->target_position.z, DoubleEq(0));
    EXPECT_THAT(eff->target_rotation.x, DoubleEq(0));
    EXPECT_THAT(eff->target_rotation.y, DoubleEq(0));
    EXPECT_THAT(eff->target_rotation.z, DoubleEq(0));
    EXPECT_THAT(eff->target_rotation.w, DoubleEq(1));
    EXPECT_THAT(eff->weight, DoubleEq(1));
    EXPECT_THAT(eff->rotation_weight, DoubleEq(1));
    EXPECT_THAT(eff->rotation_decay, DoubleEq(0.25));
    EXPECT_THAT(eff->chain_length, Eq(0u));
    EXPECT_THAT(eff->flags, Eq(0u));

    IKAPI.effector.destroy(eff);
}

TEST_F(NAME, duplicate_copies_parameters_correctly)
{
    ik_effector_t* eff = IKAPI.effector.create();
    ik_node_t* n = IKAPI.node.create(0);

    eff->node = n; /* make node not null so we can test duplicated version */
    IKAPI.vec3.set(eff->target_position.f, 1, 2, 3);
    IKAPI.quat.set(eff->target_rotation.f, 4, 5, 6, 7);
    eff->weight = 0.4;
    eff->rotation_weight = 0.8;
    eff->rotation_decay = 0.12;
    eff->chain_length = 7;
    eff->flags = IK_EFFECTOR_WEIGHT_NLERP;

    ik_effector_t* dup = IKAPI.effector.duplicate(eff);

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

    IKAPI.effector.destroy(eff);
    IKAPI.effector.destroy(dup);
    IKAPI.node.destroy(n);
}

TEST_F(NAME, attach_detach_works)
{
    ik_node_t* n = IKAPI.node.create(0);
    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.effector.attach(eff, n);

    EXPECT_THAT(eff->node, Eq(n));
    EXPECT_THAT(n->effector, Eq(eff));

    IKAPI.effector.detach(eff);

    EXPECT_THAT(eff->node, IsNull());
    EXPECT_THAT(n->effector, IsNull());

    IKAPI.effector.destroy(eff);
    IKAPI.node.destroy(n);
}

TEST_F(NAME, reattach_removes_from_previous_node)
{
    ik_node_t* n1 = IKAPI.node.create(0);
    ik_node_t* n2 = IKAPI.node.create_child(n1, 1);
    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.effector.attach(eff, n1);

    EXPECT_THAT(eff->node, Eq(n1));
    EXPECT_THAT(n1->effector, Eq(eff));

    IKAPI.effector.attach(eff, n2);

    EXPECT_THAT(n1->effector, IsNull());
    EXPECT_THAT(eff->node, Eq(n2));
    EXPECT_THAT(n2->effector, Eq(eff));

    IKAPI.node.destroy(n1);
}

TEST_F(NAME, attach_two_effectors_to_same_node)
{
    ik_node_t* n = IKAPI.node.create(0);
    ik_effector_t* eff1 = IKAPI.effector.create();
    ik_effector_t* eff2 = IKAPI.effector.create();

    EXPECT_THAT(IKAPI.effector.attach(eff1, n), Eq(IK_OK));
    EXPECT_THAT(IKAPI.effector.attach(eff2, n), Eq(IK_ALREADY_HAS_ATTACHMENT));
    EXPECT_THAT(n->effector, Eq(eff1));
    EXPECT_THAT(eff1->node, Eq(n));

    IKAPI.effector.destroy(eff2);
    IKAPI.node.destroy(n);
}

TEST_F(NAME, destroy_attached_effector)
{
    ik_node_t* n = IKAPI.node.create(0);
    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.effector.attach(eff, n);
    IKAPI.effector.destroy(eff);

    EXPECT_THAT(n->effector, IsNull());

    IKAPI.node.destroy(n);
}
