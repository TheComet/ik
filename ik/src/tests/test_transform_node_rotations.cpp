#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME transform_node_rotations

using namespace ::testing;

const double pi = std::atan(1.0) * 4;

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
        solver = ik_solver_create(IK_FABRIK);

        /*
         * The following defines a list of 3D rotations that map out a two-arm
         * tree structure that looks like this:
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
         * rl[0]-rl[6] are the local rotations, whereas rg[0]-rg[6] are the global rotations.
         */
        // Basically just typing in some random numbers here
        ik_quat_set_axis_angle(rg[0].f, 5, 2, 3, 0.1);
        ik_quat_set_axis_angle(rg[1].f, 7, 4, 1, 0.6);
        ik_quat_set_axis_angle(rg[2].f, 8, 5, 3, 0.5);
        ik_quat_set_axis_angle(rg[3].f, 5, 4, 5, 0.3);
        ik_quat_set_axis_angle(rg[4].f, 3, 2, 3, 0.8);
        ik_quat_set_axis_angle(rg[5].f, 2, 0, 2, 0.3);
        ik_quat_set_axis_angle(rg[6].f, 6, 2, 1, 0.8);

        // Figure out what the local rotations should be for the random junk above
        ik_quat_copy(rl[0].f, rg[0].f);
        // L1-L5
        for (int i = 0; i != 4; ++i)
        {
            ik_quat_copy(rl[i+1].f, rg[i+1].f);
            ik_quat_conj(rg[i].f);
            ik_quat_mul_quat(rl[i+1].f, rg[i].f);
            ik_quat_conj(rg[i].f);
        }
        // L6 is child of L3
        ik_quat_conj(rg[2].f);
        ik_quat_copy(rl[5].f, rg[5].f);
        ik_quat_mul_quat(rl[5].f, rg[2].f);
        ik_quat_conj(rg[2].f);

        // L7
        ik_quat_conj(rg[5].f);
        ik_quat_copy(rl[6].f, rg[6].f);
        ik_quat_mul_quat(rl[6].f, rg[5].f);
        ik_quat_conj(rg[5].f);
    }

    virtual void TearDown() override
    {
        ik_solver_destroy(solver);
    }

protected:
    ik_solver_t* solver;
    ik_quat_t rg[7];
    ik_quat_t rl[7];
};

TEST_F(NAME, global_to_local_single_chain)
{
    ik_node_t* n[4];
    n[0] = solver->node->create(0);
    n[1] = solver->node->create_child(n[0], 1);
    n[2] = solver->node->create_child(n[1], 2);
    n[3] = solver->node->create_child(n[2], 3);

    // Load n1-n4 with global rotations
    for (int i = 0; i != 4; ++i)
        ik_quat_copy(n[i]->rotation.f, rg[i].f);

    // Test to see if rotations match the ones we calculated during SetUp()
    const double error = 1e-15;
    ik_transform_node(n[0], IK_G2L | IK_ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rl[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rl[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rl[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rl[i].w, error)) << "Index: " << i;
    }

    // Repeat test but with different flags
    ik_transform_node(n[0], IK_L2G | IK_ROTATIONS);
    ik_transform_node(n[0], IK_G2L);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rl[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rl[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rl[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rl[i].w, error)) << "Index: " << i;
    }

    ik_transform_node(n[0], IK_L2G);
    ik_transform_node(n[0], IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rl[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rl[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rl[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rl[i].w, error)) << "Index: " << i;
    }

    // Rotations should remain unchanged if we only transform translations
    ik_transform_node(n[0], IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    ik_transform_node(n[0], IK_G2L | IK_TRANSLATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rg[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rg[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rg[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rg[i].w, error)) << "Index: " << i;
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

    // Load n1-n4 with local rotations
    for (int i = 0; i != 4; ++i)
        ik_quat_copy(n[i]->rotation.f, rl[i].f);

    // Test to see if rotations match the ones we calculated during SetUp()
    const double error = 1e-15;
    ik_transform_node(n[0], IK_L2G | IK_ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rg[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rg[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rg[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rg[i].w, error)) << "Index: " << i;
    }

    // Repeat test but with different flags
    ik_transform_node(n[0], IK_G2L | IK_ROTATIONS);
    ik_transform_node(n[0], IK_L2G);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rg[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rg[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rg[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rg[i].w, error)) << "Index: " << i;
    }

    ik_transform_node(n[0], IK_G2L);
    ik_transform_node(n[0], IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rg[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rg[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rg[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rg[i].w, error)) << "Index: " << i;
    }

    // Rotations should remain unchanged if we only transform translations
    ik_transform_node(n[0], IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    ik_transform_node(n[0], IK_L2G | IK_TRANSLATIONS);
    for (int i = 0; i != 4; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rl[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rl[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rl[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rl[i].w, error)) << "Index: " << i;
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

    // Load n1-n7 with global rotations
    for (int i = 0; i != 7; ++i)
        ik_quat_copy(n[i]->rotation.f, rg[i].f);

    // Test to see if rotations match the ones we calculated during SetUp()
    const double error = 1e-15;
    ik_transform_node(n[0], IK_G2L | IK_ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rl[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rl[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rl[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rl[i].w, error)) << "Index: " << i;
    }

    // Repeat test but with different flags
    ik_transform_node(n[0], IK_L2G | IK_ROTATIONS);
    ik_transform_node(n[0], IK_G2L);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rl[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rl[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rl[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rl[i].w, error)) << "Index: " << i;
    }

    ik_transform_node(n[0], IK_L2G);
    ik_transform_node(n[0], IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rl[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rl[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rl[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rl[i].w, error)) << "Index: " << i;
    }

    // Rotations should remain unchanged if we only transform translations
    ik_transform_node(n[0], IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    ik_transform_node(n[0], IK_G2L | IK_TRANSLATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rg[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rg[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rg[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rg[i].w, error)) << "Index: " << i;
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

    // Load n1-n4 with local rotations
    for (int i = 0; i != 7; ++i)
        ik_quat_copy(n[i]->rotation.f, rl[i].f);

    // Test to see if rotations match the ones we calculated during SetUp()
    const double error = 1e-15;
    ik_transform_node(n[0], IK_L2G | IK_ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rg[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rg[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rg[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rg[i].w, error)) << "Index: " << i;
    }

    // Repeat test but with different flags
    ik_transform_node(n[0], IK_G2L | IK_ROTATIONS);
    ik_transform_node(n[0], IK_L2G);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rg[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rg[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rg[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rg[i].w, error)) << "Index: " << i;
    }

    ik_transform_node(n[0], IK_G2L);
    ik_transform_node(n[0], IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rg[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rg[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rg[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rg[i].w, error)) << "Index: " << i;
    }

    // Rotations should remain unchanged if we only transform translations
    ik_transform_node(n[0], IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    ik_transform_node(n[0], IK_L2G | IK_TRANSLATIONS);
    for (int i = 0; i != 7; ++i)
    {
        EXPECT_THAT(n[i]->rotation.x, DoubleNear(rl[i].x, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.y, DoubleNear(rl[i].y, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.z, DoubleNear(rl[i].z, error)) << "Index: " << i;
        EXPECT_THAT(n[i]->rotation.w, DoubleNear(rl[i].w, error)) << "Index: " << i;
    }

    solver->node->destroy(n[0]);
}
