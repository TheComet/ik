#include "gmock/gmock.h"
#include "ik/ik.h"
#include "ik/node.h"
#include "ik/node_data.h"
#include "ik/refcount.h"

#define NAME node_data_flatten

using namespace ::testing;

class NAME : public Test
{
public:
    ik_node_t* tree_without_effectors()
    {
        ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9;
        ik_node_create(&tree, to_ptr(0));
        ik_node_create_child(&n1, tree, to_ptr(1));
        ik_node_create_child(&n2, n1, to_ptr(2));
        ik_node_create_child(&n3, n2, to_ptr(3));
        ik_node_create_child(&n4, n3, to_ptr(4));
        ik_node_create_child(&n5, n4, to_ptr(5));
        ik_node_create_child(&n6, n5, to_ptr(6));
        ik_node_create_child(&n7, n3, to_ptr(7));
        ik_node_create_child(&n8, n7, to_ptr(8));
        ik_node_create_child(&n9, n8, to_ptr(9));
        return tree;
    }

    ik_node_t* tree_with_two_effectors()
    {
        ik_node_t *tree, *n6, *n9;
        tree = tree_without_effectors();
        n6 = ik_node_find(tree, to_ptr(6));
        n9 = ik_node_find(tree, to_ptr(9));

        ik_effector_t *eff1, *eff2;
        ik_node_create_effector(&eff1, n6);
        ik_node_create_effector(&eff2, n9);

        return tree;
    }

    ik_node_t* tree_llr()
    {

        /*
         * e1 -> 6       8 <- e2
         *        \     /
         *         5   7
         *          \ /
         *    b2 ->  4      10 <- e3
         *            \     /
         *             3   9
         *              \ /
         *               2  <- b1
         *               |
         *               1
         *               |
         *               0
         */
        ik_node_t *tree, *b1, *b2, *e1, *e2, *e3;
        ik_node_create(&tree, to_ptr(0));
        ik_node_create_child(&b1, tree, to_ptr(1));
        ik_node_create_child(&b1, b1, to_ptr(2));
        ik_node_create_child(&b2, b1, to_ptr(3));
        ik_node_create_child(&b2, b2, to_ptr(4));
        ik_node_create_child(&e1, b2, to_ptr(5));
        ik_node_create_child(&e1, e1, to_ptr(6));
        ik_node_create_child(&e2, b2, to_ptr(7));
        ik_node_create_child(&e2, e2, to_ptr(8));
        ik_node_create_child(&e3, b1, to_ptr(9));
        ik_node_create_child(&e3, e3, to_ptr(10));

        ik_effector_t *eff1, *eff2, *eff3;
        ik_node_create_effector(&eff1, e1);
        ik_node_create_effector(&eff2, e2);
        ik_node_create_effector(&eff3, e3);

        return tree;
    }

    ik_node_t* tree_lrr()
    {
        /*
         *     e2 -> 8       10 <- e3
         *            \     /
         *             7   9
         *              \ /
         * e1 -> 4       6 <- b2
         *        \     /
         *         3   5
         *          \ /
         *           2  <- b1
         *           |
         *           1
         *           |
         *           0
         */
        ik_node_t *tree, *b1, *b2, *e1, *e2, *e3;
        ik_node_create(&tree, to_ptr(0));
        ik_node_create_child(&b1, tree, to_ptr(1));
        ik_node_create_child(&b1, b1, to_ptr(2));
        ik_node_create_child(&e1, b1, to_ptr(3));
        ik_node_create_child(&e1, e1, to_ptr(4));
        ik_node_create_child(&b2, b1, to_ptr(5));
        ik_node_create_child(&b2, b2, to_ptr(6));
        ik_node_create_child(&e2, b2, to_ptr(7));
        ik_node_create_child(&e2, e2, to_ptr(8));
        ik_node_create_child(&e3, b2, to_ptr(9));
        ik_node_create_child(&e3, e3, to_ptr(10));

        ik_effector_t *eff1, *eff2, *eff3;
        ik_node_create_effector(&eff1, e1);
        ik_node_create_effector(&eff2, e2);
        ik_node_create_effector(&eff3, e3);

        return tree;
    }

    void* to_ptr(uintptr_t i) { return (void*)i; }

    virtual void SetUp() override
    {

    }

    virtual void TearDown() override
    {
    }
};

/*
static void foo(char* p)
{

}

int main()
{
    ik_node_t *tree, *n1, *n2, *n3;
    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1, tree, to_ptr(1));
    ik_node_create_child(&n2, n1, to_ptr(2));
    ik_node_create_child(&n3, n2, to_ptr(3));
    ik_node_set_position(n1, ik_vec3_vec3(0, 1, 0));
    ik_node_set_position(n2, ik_vec3_vec3(0, 1, 0));
    ik_node_set_position(n3, ik_vec3_vec3(0, 1, 0));

    ik_effector_t *eff1, *eff2;
    ik_effector_create(&eff1);
    ik_effector_create(&eff1);
    ik_effector_set_chain_length(eff1, 2);
    ik_effector_set_chain_length(eff2, 1);
    ik_node_attach(n2, eff1);
    ik_node_attach(n3, eff2);

    ik_pole_t* pole;
    ik_pole_create(&pole, IKAPI.pole.BLENDER);
    ik_pole_set_angle(pole, 45);
    ik_node_attach(n2, pole);

    ik_constraint_t *c1, *c2;
    ik_constraint_create(&c1, IKAPI.constraint.HINGE);
    ik_constraint_create(&c2, IKAPI.constraint.CONE);
    ik_constraint_set_rotation_limits(c1, 45, 90, 0, 0, 0, 0);
    ik_constraint_set_rotation_limits(c2, -20, 20, -30, 30, 0, 0);
    ik_node_attach(tree, c1);
    ik_node_attach(n1, c2);

    ik_algorithm_t *a1, *a2;
    ik_algorithm_create(&a1, IKAPI.FABRIK);
    ik_algorithm_create(&a2, IKAPI.ONE_BONE);
    ik_algorithm_set_max_iterations(a1, 10);
    ik_algorithm_enable_features(a1, IKAPI.algorithm.TARGET_ROTATIONS);
    ik_node_attach(tree, a1);
    ik_node_attach(n2, a2);

    ik_solver_t* solver;
    ik_solver_create(&solver);
    ik_solver_prepare(solver, tree);

    while (game_is_running())
    {
        game_update_logic();
        game_update_animation();

         Update targets every frame. I'm using constant placeholder values here,
          in a game these would be actively moving
        ik_effector_set_target_position(eff1, ik_vec3_vec3(1, 2, -0.4));
        ik_effector_set_target_rotation(eff1, ik_quat_angle_vector(20, 1, 1, 0));
        ik_effector_set_target_position(eff2, ik_vec3_vec3(5, 7, 3));
        ik_pole_set_position(pole, ik_vec3_vec3(-5, 1, -5));

        ik_solver_iterate_nodes(solver, apply_scene_to_nodes_callback);
        ik_solver_update(solver);
        ik_solver_solve(solver);
        ik_solver_iterate_nodes(solver, apply_nodes_to_scene_callback);

        game_draw();
    }

    ik_solver_destroy(solver);
    ik_algorithm_destroy(a1);
    ik_algorithm_destroy(a2);
    ik_constraint_destroy(c1);
    ik_constraint_destroy(c2);
    ik_pole_destroy(pole);
    ik_effector_destroy(eff1);
    ik_effector_destroy(eff2);
    ik_node_free_recursive(tree);
}*/

TEST_F(NAME, no_action_if_tree_has_no_effectors)
{
    ik_node_t* tree = tree_without_effectors();
    ik_joblist_t joblist;
    ik_joblist_init(&joblist);
    EXPECT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_ERR_NO_EFFECTORS_FOUND));
    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, check_refcounts_are_correct)
{
    ik_joblist_t joblist;
    ik_node_t* tree = tree_with_two_effectors();

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(1));  // There should be a single flattened nda

    ik_node_data_view_t* ndv = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);

    // There are 10 nodes in the tree
    EXPECT_THAT(ndv->begin_idx, Eq(0));
    EXPECT_THAT(ndv->end_idx, Eq(10));
    EXPECT_THAT(ndv->node_data->node_count, Eq(10));

    // Each node should hold a reference, plus the ndv object itself should hold a reference
    EXPECT_THAT(ndv->node_data->refcount->refs, Eq(11));

    // All existing nodes should be pointing to the flattened nda.
    EXPECT_THAT(ndv->node_data, Eq(tree->d));
    for (int i = 0; i != 10; ++i)
        EXPECT_THAT(ndv->node_data, Eq(ik_node_find(tree, to_ptr(i))->d));

    // Attached effectors are being referenced by one nda
    EXPECT_THAT(IK_NODE_EFFECTOR(ik_node_find(tree, to_ptr(6)))->refcount->refs, Eq(1));

    ik_node_free_recursive(tree);
    EXPECT_THAT(ndv->node_data->refcount->refs, Eq(1));  // ndv structure should still hold a reference to the node data structure
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, node_tree_can_be_flattened_multiple_times)
{
    ik_joblist_t joblist1;
    ik_joblist_t joblist2;
    ik_joblist_t joblist3;
    ik_node_t* tree = tree_with_two_effectors();

    ik_joblist_init(&joblist1);
    ik_joblist_init(&joblist2);
    ik_joblist_init(&joblist3);

    ASSERT_THAT(ik_joblist_update(&joblist1, tree), Eq(IK_OK));
    ASSERT_THAT(ik_joblist_update(&joblist2, tree), Eq(IK_OK));
    ASSERT_THAT(ik_joblist_update(&joblist3, tree), Eq(IK_OK));

    ik_node_data_view_t* ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist1.ndv_list, 0);
    ik_node_data_view_t* ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist2.ndv_list, 0);
    ik_node_data_view_t* ndv3 = (ik_node_data_view_t*)vector_get_element(&joblist3.ndv_list, 0);

    // The newly created flattened node data should be the one pointing to
    // the original tree node data
    EXPECT_THAT(ndv1->node_data, Ne(tree->d));
    EXPECT_THAT(ndv2->node_data, Ne(tree->d));
    EXPECT_THAT(ndv3->node_data, Eq(tree->d));

    EXPECT_THAT(tree->d->refcount->refs, Eq(11));
    ik_joblist_deinit(&joblist1);
    EXPECT_THAT(tree->d->refcount->refs, Eq(11));
    ik_joblist_deinit(&joblist2);
    EXPECT_THAT(tree->d->refcount->refs, Eq(11));
    ik_joblist_deinit(&joblist3);
    EXPECT_THAT(tree->d->refcount->refs, Eq(10));

    ik_node_free_recursive(tree);
}

TEST_F(NAME, check_indices_are_correct)
{
    ik_joblist_t joblist;
    ik_node_t* tree = tree_with_two_effectors();

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(1));
    ik_node_data_view_t* ndv = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    ik_node_data_t* nda = ndv->node_data;

    //
    // Nodes are layed out in memory contiguously with the following
    // offsets (pre-order):
    //
    //  6           9
    //   \         /
    //    5       8
    //     \     /
    //      4   7
    //       \ /
    //        3
    //        |
    //        2
    //        |
    //        1
    //        |
    //        0
    //
    EXPECT_THAT(nda->pre_order.base_index[0], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[1], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[2], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[3], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[4], Eq(3));
    EXPECT_THAT(nda->pre_order.base_index[5], Eq(3));
    EXPECT_THAT(nda->pre_order.base_index[6], Eq(3));
    EXPECT_THAT(nda->pre_order.base_index[7], Eq(3));
    EXPECT_THAT(nda->pre_order.base_index[8], Eq(3));
    EXPECT_THAT(nda->pre_order.base_index[9], Eq(3));

    EXPECT_THAT(nda->pre_order.child_count[0], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[1], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[2], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[3], Eq(2));
    EXPECT_THAT(nda->pre_order.child_count[4], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[5], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[6], Eq(0));
    EXPECT_THAT(nda->pre_order.child_count[7], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[8], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[9], Eq(0));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, check_if_indices_are_correct_llr)
{

    ik_joblist_t joblist;
    ik_node_t* tree = tree_llr();

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ik_node_data_view_t* ndv = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    ik_node_data_t* nda = ndv->node_data;

    //
    // e1 -> 6       8 <- e2
    //        \     /
    //         5   7
    //          \ /
    //    b2 ->  4      10 <- e3
    //            \     /
    //             3   9
    //              \ /
    //               2  <- b1
    //               |
    //               1
    //               |
    //               0
    //
    EXPECT_THAT(nda->pre_order.base_index[0 ], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[1 ], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[2 ], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[3 ], Eq(2));
    EXPECT_THAT(nda->pre_order.base_index[4 ], Eq(2));
    EXPECT_THAT(nda->pre_order.base_index[5 ], Eq(4));
    EXPECT_THAT(nda->pre_order.base_index[6 ], Eq(4));
    EXPECT_THAT(nda->pre_order.base_index[7 ], Eq(4));
    EXPECT_THAT(nda->pre_order.base_index[8 ], Eq(4));
    EXPECT_THAT(nda->pre_order.base_index[9 ], Eq(2));
    EXPECT_THAT(nda->pre_order.base_index[10], Eq(2));

    EXPECT_THAT(nda->pre_order.child_count[0 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[1 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[2 ], Eq(2));
    EXPECT_THAT(nda->pre_order.child_count[3 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[4 ], Eq(2));
    EXPECT_THAT(nda->pre_order.child_count[5 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[6 ], Eq(0));
    EXPECT_THAT(nda->pre_order.child_count[7 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[8 ], Eq(0));
    EXPECT_THAT(nda->pre_order.child_count[9 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[10], Eq(0));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, check_if_indices_are_correct_lrr)
{
    ik_joblist_t joblist;
    ik_node_t* tree = tree_lrr();
    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ik_node_data_view_t* ndv = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    ik_node_data_t* nda = ndv->node_data;

    //
    //     e2 -> 8       10 <- e3
    //            \     /
    //             7   9
    //              \ /
    // e1 -> 4       6 <- b2
    //        \     /
    //         3   5
    //          \ /
    //           2  <- b1
    //           |
    //           1
    //           |
    //           0
    //
    EXPECT_THAT(nda->pre_order.base_index[0 ], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[1 ], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[2 ], Eq(0));
    EXPECT_THAT(nda->pre_order.base_index[3 ], Eq(2));
    EXPECT_THAT(nda->pre_order.base_index[4 ], Eq(2));
    EXPECT_THAT(nda->pre_order.base_index[5 ], Eq(2));
    EXPECT_THAT(nda->pre_order.base_index[6 ], Eq(2));
    EXPECT_THAT(nda->pre_order.base_index[7 ], Eq(6));
    EXPECT_THAT(nda->pre_order.base_index[8 ], Eq(6));
    EXPECT_THAT(nda->pre_order.base_index[9 ], Eq(6));
    EXPECT_THAT(nda->pre_order.base_index[10], Eq(6));

    EXPECT_THAT(nda->pre_order.child_count[0 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[1 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[2 ], Eq(2));
    EXPECT_THAT(nda->pre_order.child_count[3 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[4 ], Eq(0));
    EXPECT_THAT(nda->pre_order.child_count[5 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[6 ], Eq(2));
    EXPECT_THAT(nda->pre_order.child_count[7 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[8 ], Eq(0));
    EXPECT_THAT(nda->pre_order.child_count[9 ], Eq(1));
    EXPECT_THAT(nda->pre_order.child_count[10], Eq(0));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, ignore_effector_on_root_node)
{
    ik_joblist_t joblist;
    ik_node_t *tree, *n1, *n2;

    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1, tree, to_ptr(1));
    ik_node_create_child(&n2,  n1,  to_ptr(2));

    ik_effector_t *e1, *e2;
    ik_node_create_effector(&e1, tree);
    ik_node_create_effector(&e2, n2);

    //
    //  2 <- e2
    //  |
    //  1
    //  |
    //  0 <- e1
    //
    //

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(1));

    ik_node_data_view_t* ndv = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    ik_node_data_t* nda = ndv->node_data;
    EXPECT_THAT(nda->node_count, Eq(3));
    EXPECT_THAT(nda->user_data[0], Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(nda->user_data[2], Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(nda->effector[2], Eq(IK_NODE_EFFECTOR(n2)));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, split_trees_on_effectors)
{
    ik_joblist_t joblist;
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6;

    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1, tree, to_ptr(1));
    ik_node_create_child(&n2,  n1,  to_ptr(2));
    ik_node_create_child(&n3,  n2,  to_ptr(3));
    ik_node_create_child(&n4,  n3,  to_ptr(4));
    ik_node_create_child(&n5,  n4,  to_ptr(5));
    ik_node_create_child(&n6,  n5,  to_ptr(6));

    ik_effector_t *e1, *e2, *e3;
    ik_node_create_effector(&e1, n2);
    ik_node_create_effector(&e2, n3);
    ik_node_create_effector(&e3, n5);

    //
    //       6
    //       |
    //       5 <- e3
    //       |
    //       4
    //       |
    //       3 <- e2
    //       |
    //       2 <- e1
    //       |
    //       1
    //       |
    //       0
    //
    //

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(3));

    ik_node_data_view_t *ndv1, *ndv2, *ndv3;
    ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 1);
    ndv3 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 2);

    EXPECT_THAT(ndv1->node_data, Eq(tree->d));
    EXPECT_THAT(ndv1->node_data, Eq(n1->d));
    EXPECT_THAT(ndv1->node_data, Eq(n2->d));
    EXPECT_THAT(ndv1->node_data, Eq(n3->d));
    EXPECT_THAT(ndv1->node_data, Eq(n4->d));
    EXPECT_THAT(ndv1->node_data, Eq(n5->d));
    EXPECT_THAT(ndv1->node_data, Ne(n6->d));  // Should point to different node data
    EXPECT_THAT(ndv1->node_data->node_count, Eq(6));  // Nodes 0-5

    // Nodes 0,1,2 should be in first node data view
    EXPECT_THAT(ndv1->begin_idx, Eq(0));
    EXPECT_THAT(ndv1->end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // Nodes 2,3 should be in the second node data view
    EXPECT_THAT(ndv2->begin_idx, Eq(2));
    EXPECT_THAT(ndv2->end_idx, Eq(4));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 1), Eq(IK_NODE_USER_DATA(n3)));

    // Nodes 3,4,5 should be in the third node data view
    EXPECT_THAT(ndv3->begin_idx, Eq(3));
    EXPECT_THAT(ndv3->end_idx, Eq(6));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 0), Eq(IK_NODE_USER_DATA(n3)));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 1), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 2), Eq(IK_NODE_USER_DATA(n5)));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, split_tree_can_be_flattened_multiple_times)
{
    ik_node_t *tree, *n1, *n2, *dead3, *n4, *n5, *dead6, *dead7, *dead8, *dead9, *dead10, *dead11, *dead12, *dead13;

    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1,     tree,  to_ptr(1));
    ik_node_create_child(&n2,     n1,    to_ptr(2));
    ik_node_create_child(&dead3,  n2,    to_ptr(3));
    ik_node_create_child(&n4,     dead3, to_ptr(4));
    ik_node_create_child(&n5,     n4,    to_ptr(5));
    ik_node_create_child(&dead6,  n5,    to_ptr(6));
    ik_node_create_child(&dead7,  tree,  to_ptr(7));
    ik_node_create_child(&dead8,  n1,    to_ptr(8));
    ik_node_create_child(&dead9,  n2,    to_ptr(9));
    ik_node_create_child(&dead10, dead3, to_ptr(10));
    ik_node_create_child(&dead11, n4,    to_ptr(11));
    ik_node_create_child(&dead12, n5,    to_ptr(12));
    ik_node_create_child(&dead13, dead6, to_ptr(13));

    ik_effector_t *e1, *e2;
    ik_node_create_effector(&e1, n2);
    ik_node_create_effector(&e2, n5);
    e2->chain_length = 1;

    //
    //   13--6 <- dead
    //       |
    //   12--5 <- e2
    //       |
    //   11--4
    //       |
    //   10--3 <- dead
    //       |
    //    9--2 <- e1
    //       |
    //    8--1
    //       |
    //    7--0
    //
    //

    ik_joblist_t joblist1;
    ik_joblist_init(&joblist1);
    ASSERT_THAT(ik_joblist_update(&joblist1, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist1.ndv_list), Eq(2));
    ik_node_data_view_t* j1ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist1.ndv_list, 0);
    ik_node_data_view_t* j1ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist1.ndv_list, 1);
    EXPECT_THAT(j1ndv1->node_data->refcount->refs, Eq(7));  // nodes 0,1,2,4,5 and joblist's ndv1,ndv2 are holding a ref

    ik_joblist_t joblist2;
    ik_joblist_init(&joblist2);
    ASSERT_THAT(ik_joblist_update(&joblist2, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist2.ndv_list), Eq(2));
    ik_node_data_view_t* j2ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist2.ndv_list, 0);
    ik_node_data_view_t* j2ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist2.ndv_list, 1);
    EXPECT_THAT(j2ndv1->node_data->refcount->refs, Eq(7));  // nodes 0,1,2,4,5 and joblist's ndv1,ndv2 are holding a ref

    ik_joblist_t joblist3;
    ik_joblist_init(&joblist3);
    ASSERT_THAT(ik_joblist_update(&joblist3, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist3.ndv_list), Eq(2));
    ik_node_data_view_t* j3ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist3.ndv_list, 0);
    ik_node_data_view_t* j3ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist3.ndv_list, 1);
    EXPECT_THAT(j3ndv1->node_data->refcount->refs, Eq(7));  // nodes 0,1,2,4,5 and joblist's ndv1,ndv2 are holding a ref

    // The newly created flattened node data should be the one pointing to
    // the original tree node data
    EXPECT_THAT(j1ndv1->node_data, Ne(tree->d));
    EXPECT_THAT(j2ndv1->node_data, Ne(tree->d));
    EXPECT_THAT(j3ndv1->node_data, Eq(tree->d));
    EXPECT_THAT(j1ndv2->node_data, Ne(tree->d));
    EXPECT_THAT(j2ndv2->node_data, Ne(tree->d));
    EXPECT_THAT(j3ndv2->node_data, Eq(tree->d));

    EXPECT_THAT(j1ndv1->node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    EXPECT_THAT(j2ndv1->node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    EXPECT_THAT(j3ndv1->node_data->refcount->refs, Eq(7));  // nodes 0,1,2,4,5 and nvd1,nvd2 are holding refs
    ik_node_free_recursive(tree);
    EXPECT_THAT(j1ndv1->node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    EXPECT_THAT(j2ndv1->node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    EXPECT_THAT(j3ndv1->node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    ik_joblist_deinit(&joblist1);
    EXPECT_THAT(j2ndv1->node_data->refcount->refs, Eq(2));
    EXPECT_THAT(j3ndv1->node_data->refcount->refs, Eq(2));
    ik_joblist_deinit(&joblist2);
    EXPECT_THAT(j3ndv1->node_data->refcount->refs, Eq(2));
    ik_joblist_deinit(&joblist3);
}

TEST_F(NAME, split_trees_on_effectors_with_chain_lengths)
{
    ik_joblist_t joblist;
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6;

    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1, tree, to_ptr(1));
    ik_node_create_child(&n2,  n1,  to_ptr(2));
    ik_node_create_child(&n3,  n2,  to_ptr(3));
    ik_node_create_child(&n4,  n3,  to_ptr(4));
    ik_node_create_child(&n5,  n4,  to_ptr(5));
    ik_node_create_child(&n6,  n5,  to_ptr(6));

    ik_effector_t *e1, *e2, *e3;
    ik_node_create_effector(&e1, n2);
    ik_node_create_effector(&e2, n3);
    ik_node_create_effector(&e3, n5);
    ik_effector_set_chain_length(e3, 1);

    //
    //       6
    //       |
    //       5 <- e3 (chain length = 1)
    //       |
    //       4
    //       |
    //       3 <- e2
    //       |
    //       2 <- e1
    //       |
    //       1
    //       |
    //       0
    //
    //

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(3));

    ik_node_data_view_t *ndv1, *ndv2, *ndv3;
    ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 1);
    ndv3 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 2);

    EXPECT_THAT(ndv1->node_data, Eq(tree->d));
    EXPECT_THAT(ndv1->node_data, Eq(n1->d));
    EXPECT_THAT(ndv1->node_data, Eq(n2->d));
    EXPECT_THAT(ndv1->node_data, Eq(n3->d));
    EXPECT_THAT(ndv1->node_data, Eq(n4->d));
    EXPECT_THAT(ndv1->node_data, Eq(n5->d));
    EXPECT_THAT(ndv1->node_data, Ne(n6->d));  // Should point to different node data
    EXPECT_THAT(ndv1->node_data->node_count, Eq(6));  // Nodes 0-5

    // Nodes 0,1,2 should be in first node data view
    EXPECT_THAT(ndv1->begin_idx, Eq(0));
    EXPECT_THAT(ndv1->end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // Nodes 2,3 should be in the second node data view
    EXPECT_THAT(ndv2->begin_idx, Eq(2));
    EXPECT_THAT(ndv2->end_idx, Eq(4));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 1), Eq(IK_NODE_USER_DATA(n3)));

    // Nodes 4,5 should be in the third node data view
    EXPECT_THAT(ndv3->begin_idx, Eq(4));
    EXPECT_THAT(ndv3->end_idx, Eq(6));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 1), Eq(IK_NODE_USER_DATA(n5)));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, ignore_effector_on_root_node_with_dead_nodes)
{
    ik_joblist_t joblist;
    ik_node_t *tree, *n1, *n2, *n3, *n4;

    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1, tree, to_ptr(1));
    ik_node_create_child(&n2, n1,  to_ptr(2));
    ik_node_create_child(&n3, tree,  to_ptr(3));
    ik_node_create_child(&n4, n1,  to_ptr(4));

    ik_effector_t *e1, *e2;
    ik_node_create_effector(&e1, tree);
    ik_node_create_effector(&e2, n2);

    //
    //  4 2 <- e2
    //   \|
    //  3 1
    //   \|
    //    0 <- e1
    //
    //

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(1));

    ik_node_data_view_t* ndv = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    EXPECT_THAT(ndv->node_data->node_count, Eq(3));
    EXPECT_THAT(ndv->begin_idx, Eq(0));
    EXPECT_THAT(ndv->end_idx, Eq(3));
    EXPECT_THAT(ndv->node_data, Eq(tree->d));
    EXPECT_THAT(ndv->node_data, Eq(n1->d));
    EXPECT_THAT(ndv->node_data, Eq(n2->d));
    EXPECT_THAT(ndv->node_data, Ne(n3->d));
    EXPECT_THAT(ndv->node_data, Ne(n4->d));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, split_trees_on_effectors_with_dead_nodes)
{
    ik_joblist_t joblist;
    ik_node_t *tree, *n1, *n2, *dead3, *n4, *n5, *dead6, *dead7, *dead8, *dead9, *dead10, *dead11, *dead12, *dead13;

    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1, tree, to_ptr(1));
    ik_node_create_child(&n2, n1,  to_ptr(2));
    ik_node_create_child(&dead3, n2,  to_ptr(3));
    ik_node_create_child(&n4, dead3,  to_ptr(4));
    ik_node_create_child(&n5, n4,  to_ptr(5));
    ik_node_create_child(&dead6, n5,  to_ptr(6));
    ik_node_create_child(&dead7, tree,  to_ptr(7));
    ik_node_create_child(&dead8, n1,  to_ptr(8));
    ik_node_create_child(&dead9, n2,  to_ptr(9));
    ik_node_create_child(&dead10, dead3,  to_ptr(10));
    ik_node_create_child(&dead11, n4,  to_ptr(11));
    ik_node_create_child(&dead12, n5,  to_ptr(12));
    ik_node_create_child(&dead13, dead6,  to_ptr(13));

    ik_effector_t *e1, *e2;
    ik_node_create_effector(&e1, n2);
    ik_node_create_effector(&e2, n5);
    e2->chain_length = 1;

    //
    //   13--6 <- dead
    //       |
    //   12--5 <- e2
    //       |
    //   11--4
    //       |
    //   10--3 <- dead
    //       |
    //    9--2 <- e1
    //       |
    //    8--1
    //       |
    //    7--0
    //
    //

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(2));

    ik_node_data_view_t *ndv1, *ndv2;
    ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 1);

    // Make sure node data was created properly
    EXPECT_THAT(ndv1->node_data, Eq(tree->d));
    EXPECT_THAT(ndv1->node_data, Eq(n1->d));
    EXPECT_THAT(ndv1->node_data, Eq(n2->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead3->d));
    EXPECT_THAT(ndv1->node_data, Eq(n4->d));
    EXPECT_THAT(ndv1->node_data, Eq(n5->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead6->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead7->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead8->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead9->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead10->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead11->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead12->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead13->d));
    EXPECT_THAT(ndv1->node_data, Eq(ndv2->node_data));
    EXPECT_THAT(ndv1->node_data->node_count, Eq(5));

    // Nodes 0,1,2 should be in first node data view
    EXPECT_THAT(ndv1->begin_idx, Eq(0));
    EXPECT_THAT(ndv1->end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // Nodes 4,5 should be in the second node data view
    EXPECT_THAT(ndv2->begin_idx, Eq(3));
    EXPECT_THAT(ndv2->end_idx, Eq(5));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 1), Eq(IK_NODE_USER_DATA(n5)));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, split_trees_on_effectors_with_chain_lengths_with_dead_nodes)
{
    ik_joblist_t joblist;
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *dead6, *dead7, *dead8, *dead9, *dead10, *dead11, *dead12, *dead13;

    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1, tree, to_ptr(1));
    ik_node_create_child(&n2, n1,  to_ptr(2));
    ik_node_create_child(&n3, n2,  to_ptr(3));
    ik_node_create_child(&n4, n3,  to_ptr(4));
    ik_node_create_child(&n5, n4,  to_ptr(5));
    ik_node_create_child(&dead6, n5,  to_ptr(6));
    ik_node_create_child(&dead7, tree,  to_ptr(7));
    ik_node_create_child(&dead8, n1,  to_ptr(8));
    ik_node_create_child(&dead9, n2,  to_ptr(9));
    ik_node_create_child(&dead10, n3,  to_ptr(10));
    ik_node_create_child(&dead11, n4,  to_ptr(11));
    ik_node_create_child(&dead12, n5,  to_ptr(12));
    ik_node_create_child(&dead13, dead6,  to_ptr(13));

    ik_effector_t *e1, *e2, *e3;
    ik_node_create_effector(&e1, n2);
    ik_node_create_effector(&e2, n3);
    ik_node_create_effector(&e3, n5);
    ik_effector_set_chain_length(e3, 1);

    //
    //   13--6
    //       |
    //   12--5 <- e3 (chain length = 1)
    //       |
    //   11--4
    //       |
    //   10--3 <- e2
    //       |
    //    9--2 <- e1
    //       |
    //    8--1
    //       |
    //    7--0
    //
    //

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(3));

    ik_node_data_view_t *ndv1, *ndv2, *ndv3;
    ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 1);
    ndv3 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 2);

    // Make sure node data was created properly
    EXPECT_THAT(ndv1->node_data, Eq(tree->d));
    EXPECT_THAT(ndv1->node_data, Eq(n1->d));
    EXPECT_THAT(ndv1->node_data, Eq(n2->d));
    EXPECT_THAT(ndv1->node_data, Eq(n3->d));
    EXPECT_THAT(ndv1->node_data, Eq(n4->d));
    EXPECT_THAT(ndv1->node_data, Eq(n5->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead6->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead7->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead8->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead9->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead10->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead11->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead12->d));
    EXPECT_THAT(ndv1->node_data, Ne(dead13->d));
    EXPECT_THAT(ndv1->node_data, Eq(ndv2->node_data));
    EXPECT_THAT(ndv1->node_data, Eq(ndv3->node_data));
    EXPECT_THAT(ndv1->node_data->node_count, Eq(6));

    // Nodes 0,1,2 should be in first node data view
    EXPECT_THAT(ndv1->begin_idx, Eq(0));
    EXPECT_THAT(ndv1->end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // Nodes 2,3 should be in the second node data view
    EXPECT_THAT(ndv2->begin_idx, Eq(2));
    EXPECT_THAT(ndv2->end_idx, Eq(4));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 1), Eq(IK_NODE_USER_DATA(n3)));

    // Nodes 4,5 should be in the third node data view
    EXPECT_THAT(ndv3->begin_idx, Eq(4));
    EXPECT_THAT(ndv3->end_idx, Eq(6));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 1), Eq(IK_NODE_USER_DATA(n5)));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, split_trees_with_dead_nodes)
{
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6;
    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1, tree, to_ptr(1));
    ik_node_create_child(&n2, n1, to_ptr(2));
    ik_node_create_child(&n3, n2, to_ptr(3));
    ik_node_create_child(&n4, n3, to_ptr(4));
    ik_node_create_child(&n5, n4, to_ptr(5));
    ik_node_create_child(&n6, n5, to_ptr(6));

    ik_effector_t *e1, *e2;
    ik_effector_create(&e1);
    ik_effector_create(&e2);
    ik_node_attach_effector(n3, e1);
    ik_node_attach_effector(n6, e2);
    e2->chain_length = 1;

    ik_joblist_t joblist;
    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(2));

    //
    //                 6 <- e2, chain=1
    //                /
    // e1 -> 3       5
    //        \     /
    //         2   4 <- dead
    //          \ /
    //           1
    //           |
    //           0
    //

    struct ik_node_data_view_t* ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    struct ik_node_data_view_t* ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 1);
    ASSERT_THAT(ndv1->node_data->node_count, Eq(6));  // excludes node 4

    ASSERT_THAT(ndv1->begin_idx, Eq(0));
    ASSERT_THAT(ndv1->end_idx, Eq(4));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 3), Eq(IK_NODE_USER_DATA(n3)));

    ASSERT_THAT(ndv2->begin_idx, Eq(4));
    ASSERT_THAT(ndv2->end_idx, Eq(6));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 0), Eq(IK_NODE_USER_DATA(n5)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 1), Eq(IK_NODE_USER_DATA(n6)));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, check_joblist_order_for_disjoint_trees_llr)
{
    ik_joblist_t joblist;
    ik_node_t* tree = tree_llr();
    ik_node_t *e1, *e2, *e3;

    // Need to change effector chain lengths so tree becomes disjoint
    e1 = ik_node_find(tree, to_ptr(6));
    e2 = ik_node_find(tree, to_ptr(8));
    e3 = ik_node_find(tree, to_ptr(10));
    IK_NODE_EFFECTOR(e1)->chain_length = 1;
    IK_NODE_EFFECTOR(e2)->chain_length = 1;

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(3));

    //
    // The scenario here is nodes 0,1,2,9,10 form a chain (call this
    // "ndv 1"), nodes 5,6 form a chain ("ndv 2"), and nodes 7,8 form
    // a chain ("ndv 3"). Because nd's 2 and 3 depend on the solution of
    // nd 1, nd 1 must appear before ndv's 2 and 3 in the joblist.
    //
    // e1 -> 6       8 <- e2
    //        \     /
    //   b2 -> 5   7 <- b3
    //          \ /
    //           4      10 <- e3
    //            \     /
    //             3   9
    //              \ /
    //               2
    //               |
    //               1
    //               |
    //               0 <- b1
    //
    struct ik_node_t *b1, *b2, *b3;
    b1 = tree;
    b2 = ik_node_find(tree, to_ptr(5));  // expected base of second nd
    b3 = ik_node_find(tree, to_ptr(7));  // expected base of third nd
    struct ik_node_data_view_t* ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    struct ik_node_data_view_t* ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 1);
    struct ik_node_data_view_t* ndv3 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 2);

    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 0), Eq(IK_NODE_USER_DATA(b1)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 0), Eq(IK_NODE_USER_DATA(b2)));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 0), Eq(IK_NODE_USER_DATA(b3)));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}

TEST_F(NAME, check_joblist_order_for_disjoint_trees_llrr)
{
    ik_joblist_t joblist;
    ik_node_t *tree, *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9, *n10, *n11, *n12, *n13, *n14, *n15;

    ik_node_create(&tree, to_ptr(0));
    ik_node_create_child(&n1, tree, to_ptr(1));
    ik_node_create_child(&n2,  n1,  to_ptr(2));
    ik_node_create_child(&n3,  n2,  to_ptr(3));
    ik_node_create_child(&n4,  n3,  to_ptr(4));
    ik_node_create_child(&n5,  n4,  to_ptr(5));
    ik_node_create_child(&n6,  n5,  to_ptr(6));
    ik_node_create_child(&n7,  n6,  to_ptr(7));
    ik_node_create_child(&n8,  n5,  to_ptr(8));
    ik_node_create_child(&n9,  n8,  to_ptr(9));
    ik_node_create_child(&n10, n2,  to_ptr(10));
    ik_node_create_child(&n11, n10, to_ptr(11));
    ik_node_create_child(&n12, n11, to_ptr(12));
    ik_node_create_child(&n13, n12, to_ptr(13));
    ik_node_create_child(&n14, n11, to_ptr(14));
    ik_node_create_child(&n15, n14, to_ptr(15));

    ik_effector_t *e1, *e2, *e3, *e4, *e5;
    ik_node_create_effector(&e1, n7);
    ik_node_create_effector(&e2, n9);
    ik_node_create_effector(&e3, n13);
    ik_node_create_effector(&e4, n15);
    ik_node_create_effector(&e5, n2);
    ik_effector_set_chain_length(e1, 3);
    ik_effector_set_chain_length(e2, 1);
    ik_effector_set_chain_length(e3, 1);
    ik_effector_set_chain_length(e4, 4);

    //
    // e1 : 4,5,6,7
    // e2 : 8,9
    // e3 : 12,13
    // e4 : 2,10,11,14,15
    // e5 : 0,1,2
    //
    //     e2 -> 9
    //           |
    //           8     e3 -> 13   15 <- e4
    //            \           |  /
    // e1 -> 7--6--5         12 14
    //              \         |/
    //               4        11
    //                \      /
    //    dead node -> 3   10
    //                  \ /
    //                   2 <- e5
    //                   |
    //                   1
    //                   |
    //                   0
    //

    ik_joblist_init(&joblist);
    ASSERT_THAT(ik_joblist_update(&joblist, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&joblist.ndv_list), Eq(5));
    struct ik_node_data_view_t* ndv1 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 0);
    struct ik_node_data_view_t* ndv2 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 1);
    struct ik_node_data_view_t* ndv3 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 2);
    struct ik_node_data_view_t* ndv4 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 3);
    struct ik_node_data_view_t* ndv5 = (ik_node_data_view_t*)vector_get_element(&joblist.ndv_list, 4);

    EXPECT_THAT(ndv1->node_data->node_count, Eq(15));

    // 0,1,2
    ASSERT_THAT(ndv1->begin_idx, Eq(0));
    ASSERT_THAT(ndv1->end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(ndv1, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // 2,10,11,14,15
    ASSERT_THAT(ndv2->begin_idx, Eq(2));
    ASSERT_THAT(ndv2->end_idx, Eq(7));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 1), Eq(IK_NODE_USER_DATA(n10)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 2), Eq(IK_NODE_USER_DATA(n11)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 3), Eq(IK_NODE_USER_DATA(n14)));
    EXPECT_THAT(IK_NDV_AT(ndv2, user_data, 4), Eq(IK_NODE_USER_DATA(n15)));

    // 12,13
    ASSERT_THAT(ndv3->begin_idx, Eq(7));
    ASSERT_THAT(ndv3->end_idx, Eq(9));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 0), Eq(IK_NODE_USER_DATA(n12)));
    EXPECT_THAT(IK_NDV_AT(ndv3, user_data, 1), Eq(IK_NODE_USER_DATA(n13)));

    // 4,5,6,7
    ASSERT_THAT(ndv4->begin_idx, Eq(9));
    ASSERT_THAT(ndv4->end_idx, Eq(13));
    EXPECT_THAT(IK_NDV_AT(ndv4, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(ndv4, user_data, 1), Eq(IK_NODE_USER_DATA(n5)));
    EXPECT_THAT(IK_NDV_AT(ndv4, user_data, 2), Eq(IK_NODE_USER_DATA(n6)));
    EXPECT_THAT(IK_NDV_AT(ndv4, user_data, 3), Eq(IK_NODE_USER_DATA(n7)));

    // 8,9
    ASSERT_THAT(ndv5->begin_idx, Eq(13));
    ASSERT_THAT(ndv5->end_idx, Eq(15));
    EXPECT_THAT(IK_NDV_AT(ndv5, user_data, 0), Eq(IK_NODE_USER_DATA(n8)));
    EXPECT_THAT(IK_NDV_AT(ndv5, user_data, 1), Eq(IK_NODE_USER_DATA(n9)));

    ik_node_free_recursive(tree);
    ik_joblist_deinit(&joblist);
}
