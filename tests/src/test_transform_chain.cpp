#include "gmock/gmock.h"
#include "ik/node.h"
#include "ik/transform.h"
#include "ik/solver.h"
#include "ik/effector.h"

#define NAME transform_chain

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

    root->position = (vec3_t){1, 2, 3};
    child1->position = (vec3_t){1, 2, 3};
    child2->position = (vec3_t){1, 2, 3};
    child3->position = (vec3_t){1, 2, 3};

    ik_effector_t* eff = ik_effector_create();
    ik_node_attach_effector(child3, eff);
    vector_t effector_nodes_list;
    vector_construct(&effector_nodes_list, sizeof(ik_node_t));
    vector_push(&effector_nodes_list, &child3);

    vector_t chains;
    vector_construct(&chains, sizeof(base_chain_t));
    chain_tree_rebuild(&chains, root, &effector_nodes_list);
    vector_clear_free(&effector_nodes_list);

    ASSERT_THAT(vector_count(&chains), Eq(1u)); /* there should be 1 chain... */
    chain_t* chain = (chain_t*)vector_get_element(&chains, 0);
    ASSERT_THAT(vector_count(&chain->data.chain.nodes), Eq(4u)); /* ...consisting of 4 nodes */

    ik_chain_local_to_global(chain, TRANSFORM_ACTIVE);

    EXPECT_THAT(chain_get_node(chain, 0)->position.v.x, FloatEq(4));
    EXPECT_THAT(chain_get_node(chain, 0)->position.v.y, FloatEq(8));
    EXPECT_THAT(chain_get_node(chain, 0)->position.v.z, FloatEq(12));

    EXPECT_THAT(chain_get_node(chain, 1)->position.v.x, FloatEq(3));
    EXPECT_THAT(chain_get_node(chain, 1)->position.v.y, FloatEq(6));
    EXPECT_THAT(chain_get_node(chain, 1)->position.v.z, FloatEq(9));

    EXPECT_THAT(chain_get_node(chain, 2)->position.v.x, FloatEq(2));
    EXPECT_THAT(chain_get_node(chain, 2)->position.v.y, FloatEq(4));
    EXPECT_THAT(chain_get_node(chain, 2)->position.v.z, FloatEq(6));

    EXPECT_THAT(chain_get_node(chain, 3)->position.v.x, FloatEq(1));
    EXPECT_THAT(chain_get_node(chain, 3)->position.v.y, FloatEq(2));
    EXPECT_THAT(chain_get_node(chain, 3)->position.v.z, FloatEq(3));

    ik_chain_global_to_local(chain, TRANSFORM_ACTIVE);

    CHAIN_FOR_EACH_NODE(chain, node)
        EXPECT_THAT(node->position.v.x, FloatEq(1));
        EXPECT_THAT(node->position.v.y, FloatEq(2));
        EXPECT_THAT(node->position.v.z, FloatEq(3));
    CHAIN_END_EACH

    VECTOR_FOR_EACH(&chains, base_chain_t, base_chain)
        base_chain_destruct(base_chain);
    VECTOR_END_EACH
    vector_clear_free(&chains);
    ik_node_destroy(root);
}
