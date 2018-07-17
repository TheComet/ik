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

/*
 * The following defines list 3D coordinates that map out a two-arm tree
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
 * L1-L7 are the local positions, whereas G1-G7 are the global positions.
 */

// Global xyz positions...
#define G1X 1
#define G1Y 1.5
#define G1Z 2
#define G2X 3
#define G2Y 3
#define G2Z 3
#define G3X 5
#define G3Y -2
#define G3Z 4
#define G4X 5.5
#define G4Y 4
#define G4Z 5.5
#define G5X 6
#define G5Y 8
#define G5Z 7
#define G6X -5.5
#define G6Y 4
#define G6Z -5.5
#define G7X -6
#define G7Y 8
#define G7Z -7

#define G1 G1X,G1Y,G1Z
#define G2 G2X,G2Y,G2Z
#define G3 G3X,G3Y,G3Z
#define G4 G4X,G4Y,G4Z
#define G5 G5X,G5Y,G5Z
#define G6 G6X,G6Y,G6Z
#define G7 G7X,G7Y,G7Z

// ..and their respective local xyz positions
#define L1X 1
#define L1Y 1.5
#define L1Z 2
#define L2X 2
#define L2Y 1.5
#define L2Z 1
#define L3X 2
#define L3Y -5
#define L3Z 1
#define L4X 0.5
#define L4Y 6
#define L4Z 1.5
#define L5X 0.5
#define L5Y 4
#define L5Z 1.5
#define L6X -10.5
#define L6Y 6
#define L6Z -9.5
#define L7X -0.5
#define L7Y 4
#define L7Z -1.5

#define L1 L1X,L1Y,L1Z
#define L2 L2X,L2Y,L2Z
#define L3 L3X,L3Y,L3Z
#define L4 L4X,L4Y,L4Z
#define L5 L5X,L5Y,L5Z
#define L6 L6X,L6Y,L6Z
#define L7 L7X,L7Y,L7Z

#define ONE_TO_FOUR X(1) X(2) X(3) X(4)
#define ONE_TO_SEVEN X(1) X(2) X(3) X(4) X(5) X(6) X(7)

TEST_F(NAME, global_to_local_translations_single_chain)
{
    ik_node_t* n1 = solver->node->create(0);
    ik_node_t* n2 = solver->node->create_child(n1, 1);
    ik_node_t* n3 = solver->node->create_child(n2, 2);
    ik_node_t* n4 = solver->node->create_child(n3, 3);

    // Load positions G1-G4 into nodes
#define X(arg) (n##arg)->position = IKAPI.vec3.vec3(G##arg);
    ONE_TO_FOUR
#undef X

    IKAPI.transform.node(n1, IK_G2L | IK_TRANSLATIONS);

    // Test positions L1-L4, which are the local transforms of G1-G4
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(L##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(L##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(L##arg##Z));
    ONE_TO_FOUR
#undef X

    // Repeat test but with different flags
    IKAPI.transform.node(n1, IK_L2G | IK_TRANSLATIONS);
    IKAPI.transform.node(n1, IK_G2L);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(L##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(L##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(L##arg##Z));
    ONE_TO_FOUR
#undef X
    IKAPI.transform.node(n1, IK_L2G);
    IKAPI.transform.node(n1, IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(L##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(L##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(L##arg##Z));
    ONE_TO_FOUR
#undef X

    // Translations should remain unchanged if we only transform rotations
    IKAPI.transform.node(n1, IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    IKAPI.transform.node(n1, IK_G2L | IK_ROTATIONS);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(G##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(G##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(G##arg##Z));
    ONE_TO_FOUR
#undef X

    solver->node->destroy(n1);
}

TEST_F(NAME, local_to_global_translations_single_chain)
{
    ik_node_t* n1 = solver->node->create(0);
    ik_node_t* n2 = solver->node->create_child(n1, 1);
    ik_node_t* n3 = solver->node->create_child(n2, 2);
    ik_node_t* n4 = solver->node->create_child(n3, 3);

    // Load positions L1-L4 into nodes
#define X(arg) (n##arg)->position = IKAPI.vec3.vec3(L##arg);
    ONE_TO_FOUR
#undef X

    IKAPI.transform.node(n1, IK_L2G | IK_TRANSLATIONS);

    // Test positions G1-G4, which are the global transforms of L1-L4
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(G##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(G##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(G##arg##Z));
    ONE_TO_FOUR
#undef X

    // Repeat test but with different flags
    IKAPI.transform.node(n1, IK_G2L | IK_TRANSLATIONS);
    IKAPI.transform.node(n1, IK_L2G);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(G##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(G##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(G##arg##Z));
    ONE_TO_FOUR
#undef X
    IKAPI.transform.node(n1, IK_G2L);
    IKAPI.transform.node(n1, IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(G##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(G##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(G##arg##Z));
    ONE_TO_FOUR
#undef X

    // Translations should remain unchanged if we only transform rotations
    IKAPI.transform.node(n1, IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    IKAPI.transform.node(n1, IK_L2G | IK_ROTATIONS);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(L##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(L##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(L##arg##Z));
    ONE_TO_FOUR
#undef X

    solver->node->destroy(n1);
}

TEST_F(NAME, global_to_local_translations_two_arms)
{
    ik_node_t* n1 = solver->node->create(0);
    ik_node_t* n2 = solver->node->create_child(n1, 1);
    ik_node_t* n3 = solver->node->create_child(n2, 2);
    ik_node_t* n4 = solver->node->create_child(n3, 3);
    ik_node_t* n5 = solver->node->create_child(n4, 4);
    ik_node_t* n6 = solver->node->create_child(n3, 5);
    ik_node_t* n7 = solver->node->create_child(n6, 6);

    // Load positions G1-G7 into nodes
#define X(arg) (n##arg)->position = IKAPI.vec3.vec3(G##arg);
    ONE_TO_SEVEN
#undef X

    IKAPI.transform.node(n1, IK_G2L | IK_TRANSLATIONS);

    // Test positions L1-L7, which are the local transforms of G1-G7
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(L##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(L##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(L##arg##Z));
    ONE_TO_SEVEN
#undef X

    // Repeat test but with different flags
    IKAPI.transform.node(n1, IK_L2G | IK_TRANSLATIONS);
    IKAPI.transform.node(n1, IK_G2L);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(L##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(L##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(L##arg##Z));
    ONE_TO_SEVEN
#undef X
    IKAPI.transform.node(n1, IK_L2G);
    IKAPI.transform.node(n1, IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(L##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(L##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(L##arg##Z));
    ONE_TO_SEVEN
#undef X

    // Translations should remain unchanged if we only transform rotations
    IKAPI.transform.node(n1, IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
    IKAPI.transform.node(n1, IK_G2L | IK_ROTATIONS);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(G##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(G##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(G##arg##Z));
    ONE_TO_SEVEN
#undef X

    solver->node->destroy(n1);
}

TEST_F(NAME, local_to_global_translations_two_arms)
{
    ik_node_t* n1 = solver->node->create(0);
    ik_node_t* n2 = solver->node->create_child(n1, 1);
    ik_node_t* n3 = solver->node->create_child(n2, 2);
    ik_node_t* n4 = solver->node->create_child(n3, 3);
    ik_node_t* n5 = solver->node->create_child(n4, 4);
    ik_node_t* n6 = solver->node->create_child(n3, 5);
    ik_node_t* n7 = solver->node->create_child(n6, 6);

    // Load positions L1-L7 into nodes
#define X(arg) (n##arg)->position = IKAPI.vec3.vec3(L##arg);
    ONE_TO_SEVEN
#undef X

    IKAPI.transform.node(n1, IK_L2G | IK_TRANSLATIONS);

    // Test positions G1-G7, which are the global transforms of L1-L7
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(G##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(G##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(G##arg##Z));
    ONE_TO_SEVEN
#undef X

    // Repeat test but with different flags
    IKAPI.transform.node(n1, IK_G2L | IK_TRANSLATIONS);
    IKAPI.transform.node(n1, IK_L2G);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(G##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(G##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(G##arg##Z));
    ONE_TO_SEVEN
#undef X
    IKAPI.transform.node(n1, IK_G2L);
    IKAPI.transform.node(n1, IK_L2G | IK_TRANSLATIONS | IK_ROTATIONS);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(G##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(G##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(G##arg##Z));
    ONE_TO_SEVEN
#undef X

    // Translations should remain unchanged if we only transform rotations
    IKAPI.transform.node(n1, IK_G2L | IK_TRANSLATIONS | IK_ROTATIONS);
    IKAPI.transform.node(n1, IK_L2G | IK_ROTATIONS);
#define X(arg) \
    EXPECT_THAT((n##arg)->position.x, DoubleEq(L##arg##X)); \
    EXPECT_THAT((n##arg)->position.y, DoubleEq(L##arg##Y)); \
    EXPECT_THAT((n##arg)->position.z, DoubleEq(L##arg##Z));
    ONE_TO_SEVEN
#undef X

    solver->node->destroy(n1);
}
