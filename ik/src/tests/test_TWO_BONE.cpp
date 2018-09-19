#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME TWO_BONE

using namespace ::testing;

class NAME : public Test
{
public:
    NAME() : solver(NULL) {}

    virtual void SetUp()
    {
        IKAPI.solver.create(&solver, IKAPI.solver.TWO_BONE);
    }

    virtual void TearDown()
    {
        IKAPI.solver.destroy(solver);
    }

protected:
    ik_solver_t* solver;
};

TEST_F(NAME, reach_target_1)
{
    /*
     * This test sets up a two-bone tree with bones at 135° to one another with
     * lengths of 3. The target is positioned such that the bones will be
     * arranged at 90° when they reach the target.
     *
     * Length of 3 will catch any obvious normalization issues (which don't
     * usually appear with bone lengths of 1) and having it solve in all 3
     * dimensions will catch any obvious transform/rotation issues.
     *
     *   o
     *   |
     *   o      x   --->  o---x
     *    \               |
     *     o              o
     */
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* mid = IKAPI.node.create_child(root, 1);
    ik_node_t* tip = IKAPI.node.create_child(mid, 2);
    IKAPI.vec3.set(mid->position.f, -3/sqrt(3), 3/sqrt(3), -3/sqrt(3));  /* length 3 */
    IKAPI.vec3.set(tip->position.f, 0, 3, 0);

    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.vec3.set(eff->target_position.f, 3/sqrt(2), 3, 3/sqrt(2));
    IKAPI.effector.attach(eff, tip);

    ik.solver.set_tree(solver, root);
    ik.solver.rebuild(solver);
    ik.solver.solve(solver);

    const double error = 1e-15;

    /* Local positions should remain the same, it's the local rotations that
     * should be affected */
    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));
    EXPECT_THAT(mid->position.x, DoubleNear(-3/sqrt(3), error));
    EXPECT_THAT(mid->position.y, DoubleNear(3/sqrt(3), error));
    EXPECT_THAT(mid->position.z, DoubleNear(-3/sqrt(3), error));
    EXPECT_THAT(tip->position.x, DoubleNear(0, error));
    EXPECT_THAT(tip->position.y, DoubleNear(3, error));
    EXPECT_THAT(tip->position.z, DoubleNear(0, error));

    IKAPI.transform.node(IKAPI.solver.get_tree(solver), IKAPI.transform.L2G);

    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));
    EXPECT_THAT(mid->position.x, DoubleNear(0, error));
    EXPECT_THAT(mid->position.y, DoubleNear(3, error));
    EXPECT_THAT(mid->position.z, DoubleNear(0, error));
    EXPECT_THAT(tip->position.x, DoubleNear(3/sqrt(2), error));
    EXPECT_THAT(tip->position.y, DoubleNear(0, error));
    EXPECT_THAT(tip->position.z, DoubleNear(3/sqrt(2), error));
}

TEST_F(NAME, unreachable_1)
{
    ik_node_t* root = IKAPI.node.create(0);
    ik_node_t* mid = IKAPI.node.create_child(root, 1);
    ik_node_t* tip = IKAPI.node.create_child(mid, 2);
    IKAPI.vec3.set(mid->position.f, 0, 3, 0);
    IKAPI.vec3.set(tip->position.f, 0, 3, 0);

    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.vec3.set(eff->target_position.f, 6.1/sqrt(3), 6.1/sqrt(3), 6.1/sqrt(3));
    IKAPI.effector.attach(eff, tip);

    ik.solver.set_tree(solver, root);
    ik.solver.rebuild(solver);
    ik.solver.solve(solver);

    const double error = 1e-15;

    /* Local positions should remain the same, it's the local rotations that
     * should be affected */
    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));
    EXPECT_THAT(mid->position.x, DoubleNear(-3/sqrt(3), error));
    EXPECT_THAT(mid->position.y, DoubleNear(3/sqrt(3), error));
    EXPECT_THAT(mid->position.z, DoubleNear(-3/sqrt(3), error));
    EXPECT_THAT(tip->position.x, DoubleNear(0, error));
    EXPECT_THAT(tip->position.y, DoubleNear(3, error));
    EXPECT_THAT(tip->position.z, DoubleNear(0, error));

    IKAPI.transform.node(IKAPI.solver.get_tree(solver), IKAPI.transform.L2G);

    EXPECT_THAT(root->position.x, DoubleNear(0, error));
    EXPECT_THAT(root->position.y, DoubleNear(0, error));
    EXPECT_THAT(root->position.z, DoubleNear(0, error));
    EXPECT_THAT(mid->position.x, DoubleNear(0, error));
    EXPECT_THAT(mid->position.y, DoubleNear(3, error));
    EXPECT_THAT(mid->position.z, DoubleNear(0, error));
    EXPECT_THAT(tip->position.x, DoubleNear(3/sqrt(2), error));
    EXPECT_THAT(tip->position.y, DoubleNear(0, error));
    EXPECT_THAT(tip->position.z, DoubleNear(3/sqrt(2), error));
}
