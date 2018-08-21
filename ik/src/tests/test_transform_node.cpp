#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME transform_node

using namespace ::testing;

const double pi = std::atan(1.0) * 4;

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
        solver = ik_solver_create(IK_FABRIK);
    }

    virtual void TearDown() override
    {
        ik_solver_destroy(solver);
    }

protected:
    ik_solver_t* solver;
};

TEST_F(NAME, rotations_cause_vector_translations)
{
    ik_node_t* n1 = solver->node->create(0);
    ik_node_t* n2 = solver->node->create_child(n1, 1);
    ik_node_t* n3 = solver->node->create_child(n2, 2);

    ik_vec3_set(n1->position.f, 1, 1, 1);
    ik_vec3_set(n2->position.f, 1, 3, 1);
    ik_vec3_set(n3->position.f, 1, 6, 1);

    ik_quat_set_axis_angle(n1->rotation.f, 0, 0, 1, 45*pi/180);
    ik_quat_set_axis_angle(n2->rotation.f, 1, 0, 0, 90*pi/180);

    ik_transform_node(n1, IK_G2L);

    const double error = 1e-15;
    EXPECT_THAT(n1->position.x, DoubleNear(1, error));
    EXPECT_THAT(n1->position.y, DoubleNear(1, error));
    EXPECT_THAT(n1->position.z, DoubleNear(1, error));

    EXPECT_THAT(n2->position.x, DoubleNear(-2/sqrt(2), error));
    EXPECT_THAT(n2->position.y, DoubleNear(2/sqrt(2), error));
    EXPECT_THAT(n2->position.z, DoubleNear(0, error));

    EXPECT_THAT(n3->position.x, DoubleNear(0, error));
    EXPECT_THAT(n3->position.y, DoubleNear(0, error));
    EXPECT_THAT(n3->position.z, DoubleNear(3, error));

    solver->node->destroy(n1);
}
