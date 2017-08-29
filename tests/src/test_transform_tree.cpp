#include "gmock/gmock.h"
#include "ik/node.h"
#include "ik/transform.h"

#define NAME transform_tree

using namespace ::testing;

TEST(NAME, simple_chain_positions)
{
    ik_node_t* root = ik_node_create(0);
    ik_node_t* child1 = ik_node_create(1);
    ik_node_t* child2 = ik_node_create(2);
    ik_node_t* child3 = ik_node_create(3);
    ik_node_add_child(root, child1);
    ik_node_add_child(child1, child2);
    ik_node_add_child(child2, child3);

    child1->position = (vec3_t){1, 1, 1};
    child2->position = (vec3_t){1, 1, 1};
    child3->position = (vec3_t){1, 1, 1};

    ik_tree_local_to_global(root, TRANSFORM_ACTIVE);

    EXPECT_THAT(root->position.v.x, FloatEq(0));
    EXPECT_THAT(root->position.v.y, FloatEq(0));
    EXPECT_THAT(root->position.v.z, FloatEq(0));

    EXPECT_THAT(child1->position.v.x, FloatEq(1));
    EXPECT_THAT(child1->position.v.y, FloatEq(1));
    EXPECT_THAT(child1->position.v.z, FloatEq(1));

    EXPECT_THAT(child2->position.v.x, FloatEq(2));
    EXPECT_THAT(child2->position.v.y, FloatEq(2));
    EXPECT_THAT(child2->position.v.z, FloatEq(2));

    EXPECT_THAT(child3->position.v.x, FloatEq(3));
    EXPECT_THAT(child3->position.v.y, FloatEq(3));
    EXPECT_THAT(child3->position.v.z, FloatEq(3));

    ik_node_destroy(root);
}

TEST(NAME, simple_chain_original_positions)
{
    ik_node_t* root = ik_node_create(0);
    ik_node_t* child1 = ik_node_create(1);
    ik_node_t* child2 = ik_node_create(2);
    ik_node_t* child3 = ik_node_create(3);
    ik_node_add_child(root, child1);
    ik_node_add_child(child1, child2);
    ik_node_add_child(child2, child3);

    child1->original_position = (vec3_t){1, 1, 1};
    child2->original_position = (vec3_t){1, 1, 1};
    child3->original_position = (vec3_t){1, 1, 1};

    ik_tree_local_to_global(root, TRANSFORM_ORIGINAL);

    EXPECT_THAT(root->original_position.v.x, FloatEq(0));
    EXPECT_THAT(root->original_position.v.y, FloatEq(0));
    EXPECT_THAT(root->original_position.v.z, FloatEq(0));

    EXPECT_THAT(child1->original_position.v.x, FloatEq(1));
    EXPECT_THAT(child1->original_position.v.y, FloatEq(1));
    EXPECT_THAT(child1->original_position.v.z, FloatEq(1));

    EXPECT_THAT(child2->original_position.v.x, FloatEq(2));
    EXPECT_THAT(child2->original_position.v.y, FloatEq(2));
    EXPECT_THAT(child2->original_position.v.z, FloatEq(2));

    EXPECT_THAT(child3->original_position.v.x, FloatEq(3));
    EXPECT_THAT(child3->original_position.v.y, FloatEq(3));
    EXPECT_THAT(child3->original_position.v.z, FloatEq(3));

    ik_node_destroy(root);
}
