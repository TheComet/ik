#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME transform_chain_translations

using namespace ::testing;

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
        IKAPI.solver.create(&solver, IKAPI.solver.FABRIK);

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
        IKAPI.vec3.set(tg[0].f, 1, 1.5, 2);
        IKAPI.vec3.set(tg[1].f, 3, 3, 3);
        IKAPI.vec3.set(tg[2].f, 5, -2, 4);
        IKAPI.vec3.set(tg[3].f, 5.5, 4, 5.5);
        IKAPI.vec3.set(tg[4].f, 6, 8, 7);
        IKAPI.vec3.set(tg[5].f, -5.5, 4, -5.5);
        IKAPI.vec3.set(tg[6].f, -6, 8, -7);

        // ..and their respective local xyz positions
        IKAPI.vec3.set(tl[0].f, 1, 1.5, 2);
        IKAPI.vec3.set(tl[1].f, 2, 1.5, 1);
        IKAPI.vec3.set(tl[2].f, 2, -5, 1);
        IKAPI.vec3.set(tl[3].f, 0.5, 6, 1.5);
        IKAPI.vec3.set(tl[4].f, 0.5, 4, 1.5);
        IKAPI.vec3.set(tl[5].f, -10.5, 6, -9.5);
        IKAPI.vec3.set(tl[6].f, -0.5, 4, -1.5);
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
    n[0] = IKAPI.node.create(0);
    n[1] = IKAPI.node.create_child(n[0], 1);
    n[2] = IKAPI.node.create_child(n[1], 2);
    n[3] = IKAPI.node.create_child(n[2], 3);
    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.effector.attach(eff, n[3]);

    // Load positions tg[] into nodes
    for (int i = 0; i != 4; ++i)
        IKAPI.vec3.copy(n[i]->position.f, tg[i].f);

    IKAPI.solver.set_tree(solver, n[0]);
    IKAPI.solver.rebuild(solver);

    // Test to see if transform produces the expected local positions
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.TRANSLATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    // Repeat test but with different flags
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.TRANSLATIONS);
    ik_transform_chain_list(solver, IKAPI.transform.G2L);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    ik_transform_chain_list(solver, IKAPI.transform.L2G);
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.TRANSLATIONS | IKAPI.transform.ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    // Translations should remain unchanged if we only transform rotations
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.TRANSLATIONS | IKAPI.transform.ROTATIONS);
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }
}

TEST_F(NAME, local_to_global_single_chain)
{
    ik_node_t* n[4];
    n[0] = IKAPI.node.create(0);
    n[1] = IKAPI.node.create_child(n[0], 1);
    n[2] = IKAPI.node.create_child(n[1], 2);
    n[3] = IKAPI.node.create_child(n[2], 3);
    ik_effector_t* eff = IKAPI.effector.create();
    IKAPI.effector.attach(eff, n[3]);

    // Load positions tl[] into nodes
    for (int i = 0; i != 4; ++i)
        IKAPI.vec3.copy(n[i]->position.f, tl[i].f);

    IKAPI.solver.set_tree(solver, n[0]);
    IKAPI.solver.rebuild(solver);

    // Test to see if transform produces the expected local positions
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.TRANSLATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    // Repeat test but with different flags
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.TRANSLATIONS);
    ik_transform_chain_list(solver, IKAPI.transform.L2G);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    ik_transform_chain_list(solver, IKAPI.transform.G2L);
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.TRANSLATIONS | IKAPI.transform.ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    // Translations should remain unchanged if we only transform rotations
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.TRANSLATIONS | IKAPI.transform.ROTATIONS);
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }
}

TEST_F(NAME, global_to_local_two_arms)
{
    ik_node_t* n[7];
    n[0] = IKAPI.node.create(0);
    n[1] = IKAPI.node.create_child(n[0], 1);
    n[2] = IKAPI.node.create_child(n[1], 2);
    n[3] = IKAPI.node.create_child(n[2], 3);
    n[4] = IKAPI.node.create_child(n[3], 4);
    n[5] = IKAPI.node.create_child(n[2], 5);
    n[6] = IKAPI.node.create_child(n[5], 6);
    ik_effector_t* eff1 = IKAPI.effector.create();
    ik_effector_t* eff2 = IKAPI.effector.create();
    IKAPI.effector.attach(eff1, n[4]);
    IKAPI.effector.attach(eff2, n[6]);

    // Load positions tg[] into nodes
    for (int i = 0; i != 7; ++i)
        IKAPI.vec3.copy(n[i]->position.f, tg[i].f);

    IKAPI.solver.set_tree(solver, n[0]);
    IKAPI.solver.rebuild(solver);

    // Test to see if transform produces the expected local positions
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.TRANSLATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    // Repeat test but with different flags
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.TRANSLATIONS);
    ik_transform_chain_list(solver, IKAPI.transform.G2L);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    ik_transform_chain_list(solver, IKAPI.transform.L2G);
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.TRANSLATIONS | IKAPI.transform.ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }

    // Translations should remain unchanged if we only transform rotations
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.TRANSLATIONS | IKAPI.transform.ROTATIONS);
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }
}

TEST_F(NAME, local_to_global_two_arms)
{
    ik_node_t* n[7];
    n[0] = IKAPI.node.create(0);
    n[1] = IKAPI.node.create_child(n[0], 1);
    n[2] = IKAPI.node.create_child(n[1], 2);
    n[3] = IKAPI.node.create_child(n[2], 3);
    n[4] = IKAPI.node.create_child(n[3], 4);
    n[5] = IKAPI.node.create_child(n[2], 5);
    n[6] = IKAPI.node.create_child(n[5], 6);
    ik_effector_t* eff1 = IKAPI.effector.create();
    ik_effector_t* eff2 = IKAPI.effector.create();
    IKAPI.effector.attach(eff1, n[4]);
    IKAPI.effector.attach(eff2, n[6]);

    // Load positions tl[] into nodes
    for (int i = 0; i != 7; ++i)
        IKAPI.vec3.copy(n[i]->position.f, tl[i].f);

    IKAPI.solver.set_tree(solver, n[0]);
    IKAPI.solver.rebuild(solver);

    // Test to see if transform produces the expected local positions
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.TRANSLATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    // Repeat test but with different flags
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.TRANSLATIONS);
    ik_transform_chain_list(solver, IKAPI.transform.L2G);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    ik_transform_chain_list(solver, IKAPI.transform.G2L);
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.TRANSLATIONS | IKAPI.transform.ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tg[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tg[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tg[i].z));
    }

    // Translations should remain unchanged if we only transform rotations
    ik_transform_chain_list(solver, IKAPI.transform.G2L | IKAPI.transform.TRANSLATIONS | IKAPI.transform.ROTATIONS);
    ik_transform_chain_list(solver, IKAPI.transform.L2G | IKAPI.transform.ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->position.x, DoubleEq(tl[i].x));
        EXPECT_THAT(n[i]->position.y, DoubleEq(tl[i].y));
        EXPECT_THAT(n[i]->position.z, DoubleEq(tl[i].z));
    }
}
