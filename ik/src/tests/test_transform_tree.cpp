#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME transform_tree

using namespace ::testing;

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
        solver = IKAPI.solver.create(IK_FABRIK);
    }

    virtual void TearDown() override
    {
        IKAPI.solver.destroy(solver);
    }

protected:
    ik_solver_t* solver;
};

TEST_F(NAME, global_to_local_translations_single_chain)
{
    ik_node_t* root = solver->node->create(0);
    ik_node_t* mid1 = solver->node->create_child(root, 1);
    ik_node_t* mid2 = solver->node->create_child(mid1, 2);
    ik_node_t* tip  = solver->node->create_child(mid2, 3);

    root->position = IKAPI.vec3.vec3(1, 1.5, 2);
    mid1->position = IKAPI.vec3.vec3(3, 3, 3);
    mid2->position = IKAPI.vec3.vec3(5, -2, 4);
    tip->position = IKAPI.vec3.vec3(6, 8, 7);

    IKAPI.transform.node(root, IK_G2L | IK_TRANSLATIONS);

    EXPECT_THAT(root->position.x, DoubleEq(1));
    EXPECT_THAT(root->position.y, DoubleEq(1.5));
    EXPECT_THAT(root->position.z, DoubleEq(2));

    EXPECT_THAT(mid1->position.x, DoubleEq(2));
    EXPECT_THAT(mid1->position.y, DoubleEq(1.5));
    EXPECT_THAT(mid1->position.z, DoubleEq(1));

    EXPECT_THAT(mid2->position.x, DoubleEq(2));
    EXPECT_THAT(mid2->position.y, DoubleEq(-5));
    EXPECT_THAT(mid2->position.z, DoubleEq(1));

    EXPECT_THAT(tip->position.x, DoubleEq(1));
    EXPECT_THAT(tip->position.y, DoubleEq(10));
    EXPECT_THAT(tip->position.z, DoubleEq(3));

    solver->node->destroy(root);
}

TEST_F(NAME, local_to_global_translations_single_chain)
{
    ik_node_t* root = solver->node->create(0);
    ik_node_t* mid1 = solver->node->create_child(root, 1);
    ik_node_t* mid2 = solver->node->create_child(mid1, 2);
    ik_node_t* tip = solver->node->create_child(mid2, 3);

    root->position = IKAPI.vec3.vec3(1, 1.5, 2);
    mid1->position = IKAPI.vec3.vec3(2, 1.5, 1);
    mid2->position = IKAPI.vec3.vec3(2, -5, 1);
    tip->position = IKAPI.vec3.vec3(1, 10, 3);

    IKAPI.transform.node(root, IK_L2G | IK_TRANSLATIONS);

    EXPECT_THAT(root->position.x, DoubleEq(1));
    EXPECT_THAT(root->position.y, DoubleEq(1.5));
    EXPECT_THAT(root->position.z, DoubleEq(2));

    EXPECT_THAT(mid1->position.x, DoubleEq(3));
    EXPECT_THAT(mid1->position.y, DoubleEq(3));
    EXPECT_THAT(mid1->position.z, DoubleEq(3));

    EXPECT_THAT(mid2->position.x, DoubleEq(5));
    EXPECT_THAT(mid2->position.y, DoubleEq(-2));
    EXPECT_THAT(mid2->position.z, DoubleEq(4));

    EXPECT_THAT(tip->position.x, DoubleEq(6));
    EXPECT_THAT(tip->position.y, DoubleEq(8));
    EXPECT_THAT(tip->position.z, DoubleEq(7));

    solver->node->destroy(root);
}
