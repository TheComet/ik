#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME transform_node_rotations

using namespace ::testing;

ikreal_t* vec3p(ikreal_t x, ikreal_t y, ikreal_t z, ikreal_t buf[3])
{
    buf[0] = x;
    buf[1] = x;
    buf[2] = x;
    return buf;
}

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
        solver = IKAPI.solver.create(IK_FABRIK);

        // Basically just typing in some random numbers here
        ik_vec3_t buf;
        IKAPI.quat.set_axis_angle(&rg[0],  vec3p(5, 2, 3, buf.f), 0.1);
        IKAPI.quat.set_axis_angle(&rg[4],  vec3p(7, 4, 1, buf.f), 0.6);
        IKAPI.quat.set_axis_angle(&rg[8],  vec3p(8, 5, 3, buf.f), 0.5);
        IKAPI.quat.set_axis_angle(&rg[12], vec3p(5, 4, 5, buf.f), 0.3);
        IKAPI.quat.set_axis_angle(&rg[16], vec3p(3, 2, 3, buf.f), 0.8);
        IKAPI.quat.set_axis_angle(&rg[20], vec3p(2, 0, 2, buf.f), 0.3);
        IKAPI.quat.set_axis_angle(&rg[24], vec3p(6, 2, 1, buf.f), 0.8);

        // Figure out what the local rotations should be for the random junk above
        memcpy(&rl[0], &rg[0], sizeof(ikreal_t)*7*4);
        ik_quat_t acc_rot1, acc_rot2;
        IKAPI.quat.set(acc_rot1.f, &rg[0]);
        // L2
        IKAPI.quat.conj(acc_rot1.f);
        IKAPI.quat.mul_quat(&rl[4], &rg[0]);
        IKAPI.quat.conj(acc_rot1.f);
    }

    virtual void TearDown() override
    {
        IKAPI.solver.destroy(solver);
    }

protected:
    ik_solver_t* solver;
    ikreal_t rg[7*4];
    ikreal_t rl[7*4];
};

/*
 * The following defines list 3D rotations that map out a two-arm tree
 * structure that looks like this:
 *
 *       G5          G7
 *         \        /
 *          G4    G6
 *            \  /
 *             G3
 *             |
 *             G2
 *             |
 *             G1
 *
 * L1-L7 are the local rotations, whereas G1-G7 are the global rotations.
 */



TEST_F(NAME, global_to_local_translations_single_chain)
{
    ik_node_t* n1 = solver->node->create(0);
    ik_node_t* n2 = solver->node->create_child(n1, 1);
    ik_node_t* n3 = solver->node->create_child(n2, 2);
    ik_node_t* n4 = solver->node->create_child(n3, 3);

    solver->node->destroy(n1);
}
