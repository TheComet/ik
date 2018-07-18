#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME transform_node_translations

using namespace ::testing;

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
        solver = IKAPI.solver.create(IK_FABRIK);

        /*
         * The following lists 3D coordinates that map out a two-arm tree
         * structure that looks like this:
         *
         *       n5          n7
         *         \        /
         *          n4    n6
         *            \  /
         *             n3
         *             |
         *             n2
         *             |
         *             n1
         *
         * tl[0]-tl[6] are the local positions, whereas tg[0]-tg[6] are the global positions.
         */

        // Global xyz positions...
        tg[0] = IKAPI.vec3.vec3(1, 1.5, 2);
        tg[1] = IKAPI.vec3.vec3(3, 3, 3);
        tg[2] = IKAPI.vec3.vec3(5, -2, 4);
        tg[3] = IKAPI.vec3.vec3(5.5, 4, 5.5);
        tg[4] = IKAPI.vec3.vec3(6, 8, 7);
        tg[5] = IKAPI.vec3.vec3(-5.5, 4, -5.5);
        tg[6] = IKAPI.vec3.vec3(-6, 8, -7);

        // ..and their respective local xyz positions
        tl[0] = IKAPI.vec3.vec3(1, 1.5, 2);
        tl[1] = IKAPI.vec3.vec3(2, 1.5, 1);
        tl[2] = IKAPI.vec3.vec3(2, -5, 1);
        tl[3] = IKAPI.vec3.vec3(0.5, 6, 1.5);
        tl[4] = IKAPI.vec3.vec3(0.5, 4, 1.5);
        tl[5] = IKAPI.vec3.vec3(-10.5, 6, -9.5);
        tl[6] = IKAPI.vec3.vec3(-0.5, 4, -1.5);
    }

    virtual void TearDown() override
    {
        IKAPI.solver.destroy(solver);
    }

protected:
    ik_solver_t* solver;
    ik_vec3_t tg[7];
    ik_vec3_t tl[7];
};


TEST_F(NAME, global_to_local_single_chain)
{
    ik_node_t* n[4];
    n[0] = solver->node->create(0);
    n[1] = solver->node->create_child(n[0], 1);
    n[2] = solver->node->create_child(n[1], 2);
    n[3] = solver->node->create_child(n[2], 3);

    // Load positions tg[] into nodes
    for (int i = 0; i != 4; ++i)
        IKAPI.vec3.set(n[i]->position.f, tg[i].f);

    // Test to see if transform produces the expected local positions
    IKAPI.transform.node(n[0], IK_G2L | IK_TRANSLATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    // Repeat test but with different flags
    IKAPI.transform.node(n[0], IK_L2G | IK_TRANSLATIONS);
    IKAPI.transform.node(n[0], IK_G2L);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    IKAPI.transform.node(n[0], IK_L2G);
    IKAPI.transform.node(n[0], IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    // Translations should remain unchanged if we only transform rotations
    IKAPI.transform.node(n[0], IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    IKAPI.transform.node(n[0], IK_G2L | IK_ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    solver->node->destroy(n[0]);
}

TEST_F(NAME, local_to_global_single_chain)
{
    ik_node_t* n[4];
    n[0] = solver->node->create(0);
    n[1] = solver->node->create_child(n[0], 1);
    n[2] = solver->node->create_child(n[1], 2);
    n[3] = solver->node->create_child(n[2], 3);

    // Load positions tl[] into nodes
    for (int i = 0; i != 4; ++i)
        IKAPI.vec3.set(n[i]->position.f, tl[i].f);

    // Test to see if transform produces the expected local positions
    IKAPI.transform.node(n[0], IK_L2G | IK_TRANSLATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    // Repeat test but with different flags
    IKAPI.transform.node(n[0], IK_G2L | IK_TRANSLATIONS);
    IKAPI.transform.node(n[0], IK_L2G);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    IKAPI.transform.node(n[0], IK_G2L);
    IKAPI.transform.node(n[0], IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    // Translations should remain unchanged if we only transform rotations
    IKAPI.transform.node(n[0], IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    IKAPI.transform.node(n[0], IK_L2G | IK_ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    solver->node->destroy(n[0]);
}

TEST_F(NAME, global_to_local_two_arms)
{
    ik_node_t* n[7];
    n[0] = solver->node->create(0);
    n[1] = solver->node->create_child(n[0], 1);
    n[2] = solver->node->create_child(n[1], 2);
    n[3] = solver->node->create_child(n[2], 3);
    n[4] = solver->node->create_child(n[3], 4);
    n[5] = solver->node->create_child(n[2], 5);
    n[6] = solver->node->create_child(n[5], 6);

    // Load positions tg[] into nodes
    for (int i = 0; i != 7; ++i)
        IKAPI.vec3.set(n[i]->position.f, tg[i].f);

    // Test to see if transform produces the expected local positions
    IKAPI.transform.node(n[0], IK_G2L | IK_TRANSLATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    // Repeat test but with different flags
    IKAPI.transform.node(n[0], IK_L2G | IK_TRANSLATIONS);
    IKAPI.transform.node(n[0], IK_G2L);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    IKAPI.transform.node(n[0], IK_L2G);
    IKAPI.transform.node(n[0], IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    // Translations should remain unchanged if we only transform rotations
    IKAPI.transform.node(n[0], IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    IKAPI.transform.node(n[0], IK_G2L | IK_ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    solver->node->destroy(n[0]);
}

TEST_F(NAME, local_to_global_two_arms)
{
    ik_node_t* n[7];
    n[0] = solver->node->create(0);
    n[1] = solver->node->create_child(n[0], 1);
    n[2] = solver->node->create_child(n[1], 2);
    n[3] = solver->node->create_child(n[2], 3);
    n[4] = solver->node->create_child(n[3], 4);
    n[5] = solver->node->create_child(n[2], 5);
    n[6] = solver->node->create_child(n[5], 6);

    // Load positions tl[] into nodes
    for (int i = 0; i != 7; ++i)
        IKAPI.vec3.set(n[i]->position.f, tl[i].f);

    // Test to see if transform produces the expected local positions
    IKAPI.transform.node(n[0], IK_L2G | IK_TRANSLATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    // Repeat test but with different flags
    IKAPI.transform.node(n[0], IK_G2L | IK_TRANSLATIONS);
    IKAPI.transform.node(n[0], IK_L2G);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    IKAPI.transform.node(n[0], IK_G2L);
    IKAPI.transform.node(n[0], IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    // Translations should remain unchanged if we only transform rotations
    IKAPI.transform.node(n[0], IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    IKAPI.transform.node(n[0], IK_L2G | IK_ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    solver->node->destroy(n[0]);
}
