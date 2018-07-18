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
        solver = IKAPI.solver.create(IK_FABRIK);
    }

    virtual void TearDown() override
    {
        IKAPI.solver.destroy(solver);
    }

protected:
    ik_solver_t* solver;
};

static ikreal_t* vec3p(ikreal_t x, ikreal_t y, ikreal_t z, ikreal_t buf[3])
{
    buf[0] = x;
    buf[1] = y;
    buf[2] = z;
    return buf;
}

TEST_F(NAME, rotations_cause_vector_translations)
{
    ik_node_t* n1 = solver->node->create(0);
    ik_node_t* n2 = solver->node->create_child(n1, 1);
    ik_node_t* n3 = solver->node->create_child(n2, 2);

    n1->position = IKAPI.vec3.vec3(1, 1, 1);
    n2->position = IKAPI.vec3.vec3(1, 3, 1);
    n3->position = IKAPI.vec3.vec3(1, 6, 1);

    ik_vec3_t buf;
    IKAPI.quat.set_axis_angle(n1->rotation.f, vec3p(0, 0, 1, buf.f), 45*pi/180);
    IKAPI.quat.set_axis_angle(n2->rotation.f, vec3p(1, 0, 0, buf.f), 90*pi/180);

    IKAPI.transform.node(n1, IK_G2L);

    EXPECT_THAT(n1->position.x, DoubleEq(1));
    EXPECT_THAT(n1->position.y, DoubleEq(1));
    EXPECT_THAT(n1->position.z, DoubleEq(1));

    EXPECT_THAT(n2->position.x, DoubleEq(-2/sqrt(2)));
    EXPECT_THAT(n2->position.y, DoubleEq(2/sqrt(2)));
    EXPECT_THAT(n2->position.z, DoubleEq(0));

    EXPECT_THAT(n3->position.x, DoubleEq(0));
    EXPECT_THAT(n3->position.y, DoubleEq(0));
    EXPECT_THAT(n3->position.z, DoubleEq(-3));

    solver->node->destroy(n1);
}
