#include "gmock/gmock.h"
#include "ik/algorithm.h"
#include "ik/node.h"
#include "ik/solvers.h"

#define NAME solvers

using namespace ::testing;

struct ik_solver
{
    IK_SOLVER_HEAD
};

static int  dummy_init(struct ik_solver* solver) { return 0; }
static void dummy_deinit(struct ik_solver* solver) {}
static int  dummy_rebuild(struct ik_solver* solver, const struct ik_subtree* subtree) { return 0; }
static void update_translations(struct ik_solver* solver) {}
static int  dummy_solve(struct ik_solver* solver) { return 0; }
static void dummy_iterate_nodes(const struct ik_solver* solver, ik_solver_callback_func cb) {}

const struct ik_solver_interface ik_solver_DUMMY = {
    "dummy",
    sizeof(struct ik_solver),
    dummy_init,
    dummy_deinit,
    dummy_rebuild,
    update_translations,
    dummy_solve,
    dummy_iterate_nodes
};

class NAME : public Test
{
public:
    ik_node* tree_without_effectors()
    {
        ik_node* tree = ik_node_create(ik_guid(0));
        ik_node* n1 = ik_node_create_child(tree, ik_guid(1));
        ik_node* n2 = ik_node_create_child(n1, ik_guid(2));
        ik_node* n3 = ik_node_create_child(n2, ik_guid(3));
        ik_node* n4 = ik_node_create_child(n3, ik_guid(4));
        ik_node* n5 = ik_node_create_child(n4, ik_guid(5));
        ik_node* n6 = ik_node_create_child(n5, ik_guid(6));
        ik_node* n7 = ik_node_create_child(n3, ik_guid(7));
        ik_node* n8 = ik_node_create_child(n7, ik_guid(8));
        ik_node* n9 = ik_node_create_child(n8, ik_guid(9));
        return tree;
    }

    ik_node* tree_with_two_effectors()
    {
        ik_node *tree, *n6, *n9;
        tree = tree_without_effectors();
        n6 = ik_node_find(tree, ik_guid(6));
        n9 = ik_node_find(tree, ik_guid(9));

        ik_node_create_effector(n6);
        ik_node_create_effector(n9);
        ik_node_create_algorithm(tree, "dummy");

        return tree;
    }

    ik_node* tree_with_two_effectors_and_no_algorithms()
    {
        ik_node *tree, *n6, *n9;
        tree = tree_without_effectors();
        n6 = ik_node_find(tree, ik_guid(6));
        n9 = ik_node_find(tree, ik_guid(9));

        ik_node_create_effector(n6);
        ik_node_create_effector(n9);

        return tree;
    }

    ik_node* tree_llr()
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
        ik_node* tree = ik_node_create(ik_guid(0));
        ik_node* b1 = ik_node_create_child(tree, ik_guid(1));
        b1 = ik_node_create_child(b1, ik_guid(2));
        ik_node* b2 = ik_node_create_child(b1, ik_guid(3));
        b2 = ik_node_create_child(b2, ik_guid(4));
        ik_node* e1 = ik_node_create_child(b2, ik_guid(5));
        e1 = ik_node_create_child(e1, ik_guid(6));
        ik_node* e2 = ik_node_create_child(b2, ik_guid(7));
        e2 = ik_node_create_child(e2, ik_guid(8));
        ik_node* e3 = ik_node_create_child(b1, ik_guid(9));
        e3 = ik_node_create_child(e3, ik_guid(10));

        ik_node_create_effector(e1);
        ik_node_create_effector(e2);
        ik_node_create_effector(e3);

        return tree;
    }

    ik_node* tree_lrr()
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
        ik_node* tree = ik_node_create(ik_guid(0));
        ik_node* b1 = ik_node_create_child(tree, ik_guid(1));
        b1 = ik_node_create_child(b1, ik_guid(2));
        ik_node* e1 = ik_node_create_child(b1, ik_guid(3));
        e1 = ik_node_create_child(e1, ik_guid(4));
        ik_node* b2 = ik_node_create_child(b1, ik_guid(5));
        b2 = ik_node_create_child(b2, ik_guid(6));
        ik_node* e2 = ik_node_create_child(b2, ik_guid(7));
        e2 = ik_node_create_child(e2, ik_guid(8));
        ik_node* e3 = ik_node_create_child(b2, ik_guid(9));
        e3 = ik_node_create_child(e3, ik_guid(10));

        ik_node_create_effector(e1);
        ik_node_create_effector(e2);
        ik_node_create_effector(e3);

        return tree;
    }

    virtual void SetUp() override
    {
        ik_solver_register(&ik_solver_DUMMY);
    }

    virtual void TearDown() override
    {
        ik_solver_unregister(&ik_solver_DUMMY);
    }
};

/*
static void foo(char* p)
{

}

int main()
{
    ik_node *tree, *n1, *n2, *n3;
    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(tree, ik_guid(1));
    ik_node* n2 = ik_node_create_child(n1, ik_guid(2));
    ik_node* n3 = ik_node_create_child(n2, ik_guid(3));
    ik_node_set_position(n1, ik_vec3_vec3(0, 1, 0));
    ik_node_set_position(n2, ik_vec3_vec3(0, 1, 0));
    ik_node_set_position(n3, ik_vec3_vec3(0, 1, 0));

    ik_effector *eff1, *eff2;
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

    ik_solver* solver;
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

TEST_F(NAME, no_action_if_tree_has_no_effectors_or_algorithms)
{
    ik_node* tree = tree_without_effectors();
    ik_solvers* solvers = ik_solvers_create(tree);
    EXPECT_THAT(solvers, IsNull());
    IK_DECREF(tree);
}

TEST_F(NAME, no_action_if_tree_has_no_algorithms)
{
    ik_node* tree = tree_without_effectors();
    ik_node* n6 = ik_node_find(tree, ik_guid(6));
    ik_node* n9 = ik_node_find(tree, ik_guid(9));

    ik_effector* eff1 = ik_node_create_effector(n6);
    ik_effector* eff2 = ik_node_create_effector(n9);

    ik_solvers* solvers = ik_solvers_create(tree);
    EXPECT_THAT(solvers, IsNull());

    IK_DECREF(tree);
}


TEST_F(NAME, check_refcounts_are_correct)
{
    ik_node* tree = tree_with_two_effectors();
    ik_solvers* solvers = ik_solvers_create(tree);
    ASSERT_THAT(solvers, NotNull());
    ASSERT_THAT(vector_count(&solvers->solver_list), Eq(1));  // There should be one FABRIK solver

    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers->solver_list, 0);
    ASSERT_THAT(solver->algorithm, NotNull());
    EXPECT_THAT(solver->algorithm->name, StrEq("dummy"));

    EXPECT_THAT(solvers->refcount->refs, Eq(1));
    IK_DECREF(tree);
    IK_DECREF(solvers);
}

/*
TEST_F(NAME, node_tree_can_be_flattened_multiple_times)
{
    ik_solvers solvers1;
    ik_solvers solvers2;
    ik_solvers solvers3;
    ik_node* tree = tree_with_two_effectors();

    ik_solvers_init(&solvers1);
    ik_solvers_init(&solvers2);
    ik_solvers_init(&solvers3);

    ASSERT_THAT(ik_solvers_update(&solvers1, tree), Eq(IK_OK));
    ASSERT_THAT(ik_solvers_update(&solvers2, tree), Eq(IK_OK));
    ASSERT_THAT(ik_solvers_update(&solvers3, tree), Eq(IK_OK));

    ik_solver* solver1 = *(ik_solver**)vector_get_element(&solvers1.solver_list, 0);
    ik_solver* solver2 = *(ik_solver**)vector_get_element(&solvers2.solver_list, 0);
    ik_solver* solver3 = *(ik_solver**)vector_get_element(&solvers3.solver_list, 0);

    // The newly created flattened node data should be the one pointing to
    // the original tree node data
    EXPECT_THAT(solver1->ndv.node_data, Ne(tree->d));
    EXPECT_THAT(solver2->ndv.node_data, Ne(tree->d));
    EXPECT_THAT(solver3->ndv.node_data, Eq(tree->d));

    EXPECT_THAT(tree->d->refcount->refs, Eq(11));
    ik_solvers_deinit(&solvers1);
    EXPECT_THAT(tree->d->refcount->refs, Eq(11));
    ik_solvers_deinit(&solvers2);
    EXPECT_THAT(tree->d->refcount->refs, Eq(11));
    ik_solvers_deinit(&solvers3);
    EXPECT_THAT(tree->d->refcount->refs, Eq(10));

    ik_node_free_recursive(tree);
}

TEST_F(NAME, choose_algorithm_closest_to_root)
{
    ik_solvers solvers;
    ik_node* tree = tree_with_two_effectors_and_no_algorithms();
    ik_node* n3 = ik_node_find(tree, ik_guid(3));

    ik_algorithm_t *a1, *a2;
    ik_node_create_algorithm(&a1, tree);
    ik_node_create_algorithm(&a2, n3);
    a1->type = IK_SOLVER_DUMMY1;
    a2->type = IK_SOLVER_DUMMY2;

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));

    //
    // e1 -> 6           9 <- e2
    //        \         /
    //         5       8
    //          \     /
    //           4   7
    //            \ /
    //             3 <- a2
    //             |
    //             2
    //             |
    //             1
    //             |
    //             0 <- a1
    //

    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(1));

    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    EXPECT_THAT(IK_NDV_AT(&solver->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(solver->ndv.node_data->node_count, Eq(10));
    EXPECT_THAT(solver->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver->ndv.chain_end_idx, Eq(10));
    EXPECT_THAT(solver->algorithm, Eq(a1));

    ik_solvers_deinit(&solvers);
    ik_node_free_recursive(tree);
}

TEST_F(NAME, choose_algorithm_closest_to_root_with_limited_chain_length)
{
    ik_solvers solvers;
    ik_node* tree = tree_with_two_effectors_and_no_algorithms();
    ik_node* n2 = ik_node_find(tree, ik_guid(2));
    ik_node* n3 = ik_node_find(tree, ik_guid(3));
    ik_node* n6 = ik_node_find(tree, ik_guid(6));
    ik_node* n9 = ik_node_find(tree, ik_guid(9));

    IK_NODE_EFFECTOR(n6)->chain_length = 4;
    IK_NODE_EFFECTOR(n9)->chain_length = 4;

    ik_algorithm_t *a1, *a2;
    ik_node_create_algorithm(&a1, tree);
    ik_node_create_algorithm(&a2, n3);
    a1->type = IK_SOLVER_DUMMY1;
    a2->type = IK_SOLVER_DUMMY2;

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));

    //
    // e1 -> 6           9 <- e2
    //        \         /
    //         5       8
    //          \     /
    //           4   7
    //            \ /
    //             3 <- a2
    //             |
    //             2 <- chains end here
    //             |
    //             1
    //             |
    //             0 <- a1
    //

    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(1));

    // We expect algorithm 1 to be chosen because it is the next available
    // one after the chains end
    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    EXPECT_THAT(IK_NDV_AT(&solver->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(solver->ndv.node_data->node_count, Eq(8));
    EXPECT_THAT(solver->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver->ndv.chain_end_idx, Eq(8));
    EXPECT_THAT(solver->algorithm, Eq(a1));

    ik_solvers_deinit(&solvers);
    ik_node_free_recursive(tree);
}

TEST_F(NAME, choose_algorithm_closest_to_end_of_chain_exact)
{
    ik_solvers solvers;
    ik_node* tree = tree_with_two_effectors_and_no_algorithms();
    ik_node* n3 = ik_node_find(tree, ik_guid(3));
    ik_node* n6 = ik_node_find(tree, ik_guid(6));
    ik_node* n9 = ik_node_find(tree, ik_guid(9));

    IK_NODE_EFFECTOR(n6)->chain_length = 3;
    IK_NODE_EFFECTOR(n9)->chain_length = 3;

    ik_algorithm_t *a1, *a2;
    ik_node_create_algorithm(&a1, tree);
    ik_node_create_algorithm(&a2, n3);
    a1->type = IK_SOLVER_DUMMY1;
    a2->type = IK_SOLVER_DUMMY2;

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));

    //
    // e1 -> 6           9 <- e2
    //        \         /
    //         5       8
    //          \     /
    //           4   7
    //            \ /
    //             3 <- a2, chains end here
    //             |
    //             2
    //             |
    //             1
    //             |
    //             0 <- a1
    //

    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(1));

    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    EXPECT_THAT(IK_NDV_AT(&solver->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n3)));
    EXPECT_THAT(solver->ndv.node_data->node_count, Eq(7));
    EXPECT_THAT(solver->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver->ndv.chain_end_idx, Eq(7));
    EXPECT_THAT(solver->algorithm, Eq(a2));

    ik_solvers_deinit(&solvers);
    ik_node_free_recursive(tree);
}

TEST_F(NAME, choose_algorithm_closest_to_end_of_chain)
{
    ik_solvers solvers;
    ik_node* tree = tree_with_two_effectors_and_no_algorithms();
    ik_node* n3 = ik_node_find(tree, ik_guid(3));
    ik_node* n4 = ik_node_find(tree, ik_guid(4));
    ik_node* n6 = ik_node_find(tree, ik_guid(6));
    ik_node* n7 = ik_node_find(tree, ik_guid(7));
    ik_node* n9 = ik_node_find(tree, ik_guid(9));

    IK_NODE_EFFECTOR(n6)->chain_length = 2;
    IK_NODE_EFFECTOR(n9)->chain_length = 2;

    ik_algorithm_t *a1, *a2;
    ik_node_create_algorithm(&a1, tree);
    ik_node_create_algorithm(&a2, n3);
    a1->type = IK_SOLVER_DUMMY1;
    a2->type = IK_SOLVER_DUMMY2;

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));

    //
    // e1 -> 6           9 <- e2
    //        \         /
    //         5       8
    //          \     /
    //        -> 4   7 <- chains end here
    //            \ /
    //             3 <- a2
    //             |
    //             2
    //             |
    //             1
    //             |
    //             0 <- a1
    //

    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(2));

    ik_solver* solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    ik_solver* solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);
    EXPECT_THAT(solver1->ndv.node_data, Eq(solver2->ndv.node_data));
    EXPECT_THAT(solver1->ndv.node_data->node_count, Eq(6));

    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(solver1->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver1->ndv.chain_end_idx, Eq(3));
    EXPECT_THAT(solver1->algorithm, Eq(a2));

    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n7)));
    EXPECT_THAT(solver2->ndv.subbase_idx, Eq(3));
    EXPECT_THAT(solver2->ndv.chain_begin_idx, Eq(4));
    EXPECT_THAT(solver2->ndv.chain_end_idx, Eq(6));
    EXPECT_THAT(solver2->algorithm, Eq(a2));

    ik_solvers_deinit(&solvers);
    ik_node_free_recursive(tree);
}

TEST_F(NAME, split_tree_with_different_algorithms_1)
{
    ik_solvers solvers;
    ik_node* tree = tree_with_two_effectors_and_no_algorithms();
    ik_node* n2 = ik_node_find(tree, ik_guid(2));
    ik_node* n3 = ik_node_find(tree, ik_guid(3));
    ik_node* n4 = ik_node_find(tree, ik_guid(4));
    ik_node* n5 = ik_node_find(tree, ik_guid(5));
    ik_node* n6 = ik_node_find(tree, ik_guid(6));
    ik_node* n7 = ik_node_find(tree, ik_guid(7));
    ik_node* n8 = ik_node_find(tree, ik_guid(8));
    ik_node* n9 = ik_node_find(tree, ik_guid(9));

    IK_NODE_EFFECTOR(n6)->chain_length = 4;
    IK_NODE_EFFECTOR(n9)->chain_length = 3;

    ik_algorithm_t *a1, *a2;
    ik_node_create_algorithm(&a1, tree);
    ik_node_create_algorithm(&a2, n3);
    a1->type = IK_SOLVER_DUMMY1;
    a2->type = IK_SOLVER_DUMMY2;

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));

    //
    // chain=4
    // e1 -> 6           9 <- e2, chain=3
    //        \         /
    //         5       8
    //          \     /
    //           4   7
    //            \ /
    //             3 <- a2
    //             |
    //             2
    //             |
    //             1
    //             |
    //             0 <- a1
    //

    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(2));
    ik_solver* solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    ik_solver* solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);

    EXPECT_THAT(solver1->ndv.node_data, Eq(solver2->ndv.node_data));
    EXPECT_THAT(solver1->ndv.node_data->node_count, Eq(8));

    EXPECT_THAT(solver1->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver1->ndv.chain_end_idx, Eq(5));
    EXPECT_THAT(solver1->algorithm, Eq(a1));

    EXPECT_THAT(solver2->ndv.subbase_idx, Eq(1));
    EXPECT_THAT(solver2->ndv.chain_begin_idx, Eq(5));
    EXPECT_THAT(solver2->ndv.chain_end_idx, Eq(8));
    EXPECT_THAT(solver2->algorithm, Eq(a2));

    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n3)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 3), Eq(IK_NODE_USER_DATA(n5)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 4), Eq(IK_NODE_USER_DATA(n6)));

    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 5), Eq(IK_NODE_USER_DATA(n3)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 6), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 7), Eq(IK_NODE_USER_DATA(n5)));

    ik_solvers_deinit(&solvers);
    ik_node_free_recursive(tree);
}

TEST_F(NAME, split_tree_with_different_algorithms_2)
{
    ik_solvers solvers;
    ik_node* tree = tree_with_two_effectors_and_no_algorithms();
    ik_node* n1 = ik_node_find(tree, ik_guid(1));
    ik_node* n3 = ik_node_find(tree, ik_guid(3));
    ik_node* n4 = ik_node_find(tree, ik_guid(4));
    ik_node* n6 = ik_node_find(tree, ik_guid(6));
    ik_node* n9 = ik_node_find(tree, ik_guid(9));

    IK_NODE_EFFECTOR(n6)->chain_length = 2;
    IK_NODE_EFFECTOR(n9)->chain_length = 5;

    ik_algorithm_t *a1, *a2;
    ik_node_create_algorithm(&a1, tree);
    ik_node_create_algorithm(&a2, n3);
    a1->type = IK_SOLVER_DUMMY1;
    a2->type = IK_SOLVER_DUMMY2;

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));

    //
    // chain=2
    // e1 -> 6           9 <- e2, chain=5
    //        \         /
    //         5       8
    //          \     /
    //           4   7
    //            \ /
    //             3 <- a2
    //             |
    //             2
    //             |
    //             1
    //             |
    //             0 <- a1
    //

    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(2));
    ik_solver* solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    ik_solver* solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);

    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(solver1->ndv.node_data->node_count, Eq(9));
    EXPECT_THAT(solver1->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver1->ndv.chain_end_idx, Eq(6));
    EXPECT_THAT(solver1->algorithm, Eq(a1));

    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(solver2->ndv.subbase_idx, Eq(6));
    EXPECT_THAT(solver2->ndv.chain_begin_idx, Eq(7));
    EXPECT_THAT(solver2->ndv.chain_end_idx, Eq(9));
    EXPECT_THAT(solver2->algorithm, Eq(a1));

    ik_solvers_deinit(&solvers);
    ik_node_free_recursive(tree);
}

TEST_F(NAME, algorithm_terminates_chain)
{
    ik_solvers solvers;
    ik_node* tree = tree_with_two_effectors_and_no_algorithms();
    ik_node* n2 = ik_node_find(tree, ik_guid(2));
    ik_node* n3 = ik_node_find(tree, ik_guid(3));

    ik_algorithm_t *a1, *a2;
    ik_node_create_algorithm(&a1, n2);
    ik_node_create_algorithm(&a2, n3);
    a1->type = IK_SOLVER_DUMMY1;
    a2->type = IK_SOLVER_DUMMY2;

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));

    //
    // Nodes are layed out in memory contiguously with the following
    // offsets (pre-order):
    //
    // e1 -> 6           9 <- e2
    //        \         /
    //         5       8
    //          \     /
    //           4   7
    //            \ /
    //             3 <- a2
    //             |
    //             2 <- a1
    //             |
    //             1
    //             |
    //             0
    //

    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(1));

    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    EXPECT_THAT(IK_NDV_AT(&solver->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(solver->ndv.node_data->node_count, Eq(8));
    EXPECT_THAT(solver->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver->ndv.chain_end_idx, Eq(8));
    EXPECT_THAT(solver->algorithm, Eq(a1));

    ik_solvers_deinit(&solvers);
    ik_node_free_recursive(tree);
}

TEST_F(NAME, check_indices_are_correct)
{
    ik_solvers solvers;
    ik_node* tree = tree_with_two_effectors();

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(1));
    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    ik_node_data_t* nda = solver->ndv.node_data;

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
    EXPECT_THAT(nda->base_idx[0], Eq(0));
    EXPECT_THAT(nda->base_idx[1], Eq(0));
    EXPECT_THAT(nda->base_idx[2], Eq(0));
    EXPECT_THAT(nda->base_idx[3], Eq(0));
    EXPECT_THAT(nda->base_idx[4], Eq(3));
    EXPECT_THAT(nda->base_idx[5], Eq(3));
    EXPECT_THAT(nda->base_idx[6], Eq(3));
    EXPECT_THAT(nda->base_idx[7], Eq(3));
    EXPECT_THAT(nda->base_idx[8], Eq(3));
    EXPECT_THAT(nda->base_idx[9], Eq(3));

    EXPECT_THAT(nda->child_count[0], Eq(1));
    EXPECT_THAT(nda->child_count[1], Eq(1));
    EXPECT_THAT(nda->child_count[2], Eq(1));
    EXPECT_THAT(nda->child_count[3], Eq(2));
    EXPECT_THAT(nda->child_count[4], Eq(1));
    EXPECT_THAT(nda->child_count[5], Eq(1));
    EXPECT_THAT(nda->child_count[6], Eq(0));
    EXPECT_THAT(nda->child_count[7], Eq(1));
    EXPECT_THAT(nda->child_count[8], Eq(1));
    EXPECT_THAT(nda->child_count[9], Eq(0));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, check_if_indices_are_correct_llr)
{

    ik_solvers solvers;
    ik_node* tree = tree_llr();

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    ik_node_data_t* nda = solver->ndv.node_data;

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
    EXPECT_THAT(nda->base_idx[0 ], Eq(0));
    EXPECT_THAT(nda->base_idx[1 ], Eq(0));
    EXPECT_THAT(nda->base_idx[2 ], Eq(0));
    EXPECT_THAT(nda->base_idx[3 ], Eq(2));
    EXPECT_THAT(nda->base_idx[4 ], Eq(2));
    EXPECT_THAT(nda->base_idx[5 ], Eq(4));
    EXPECT_THAT(nda->base_idx[6 ], Eq(4));
    EXPECT_THAT(nda->base_idx[7 ], Eq(4));
    EXPECT_THAT(nda->base_idx[8 ], Eq(4));
    EXPECT_THAT(nda->base_idx[9 ], Eq(2));
    EXPECT_THAT(nda->base_idx[10], Eq(2));

    EXPECT_THAT(nda->child_count[0 ], Eq(1));
    EXPECT_THAT(nda->child_count[1 ], Eq(1));
    EXPECT_THAT(nda->child_count[2 ], Eq(2));
    EXPECT_THAT(nda->child_count[3 ], Eq(1));
    EXPECT_THAT(nda->child_count[4 ], Eq(2));
    EXPECT_THAT(nda->child_count[5 ], Eq(1));
    EXPECT_THAT(nda->child_count[6 ], Eq(0));
    EXPECT_THAT(nda->child_count[7 ], Eq(1));
    EXPECT_THAT(nda->child_count[8 ], Eq(0));
    EXPECT_THAT(nda->child_count[9 ], Eq(1));
    EXPECT_THAT(nda->child_count[10], Eq(0));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, check_if_indices_are_correct_lrr)
{
    ik_solvers solvers;
    ik_node* tree = tree_lrr();
    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    ik_node_data_t* nda = solver->ndv.node_data;

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
    EXPECT_THAT(nda->base_idx[0 ], Eq(0));
    EXPECT_THAT(nda->base_idx[1 ], Eq(0));
    EXPECT_THAT(nda->base_idx[2 ], Eq(0));
    EXPECT_THAT(nda->base_idx[3 ], Eq(2));
    EXPECT_THAT(nda->base_idx[4 ], Eq(2));
    EXPECT_THAT(nda->base_idx[5 ], Eq(2));
    EXPECT_THAT(nda->base_idx[6 ], Eq(2));
    EXPECT_THAT(nda->base_idx[7 ], Eq(6));
    EXPECT_THAT(nda->base_idx[8 ], Eq(6));
    EXPECT_THAT(nda->base_idx[9 ], Eq(6));
    EXPECT_THAT(nda->base_idx[10], Eq(6));

    EXPECT_THAT(nda->child_count[0 ], Eq(1));
    EXPECT_THAT(nda->child_count[1 ], Eq(1));
    EXPECT_THAT(nda->child_count[2 ], Eq(2));
    EXPECT_THAT(nda->child_count[3 ], Eq(1));
    EXPECT_THAT(nda->child_count[4 ], Eq(0));
    EXPECT_THAT(nda->child_count[5 ], Eq(1));
    EXPECT_THAT(nda->child_count[6 ], Eq(2));
    EXPECT_THAT(nda->child_count[7 ], Eq(1));
    EXPECT_THAT(nda->child_count[8 ], Eq(0));
    EXPECT_THAT(nda->child_count[9 ], Eq(1));
    EXPECT_THAT(nda->child_count[10], Eq(0));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, ignore_effector_on_root_node)
{
    ik_solvers solvers;
    ik_node *tree, *n1, *n2;

    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(tree, ik_guid(1));
    ik_node* n2 = ik_node_create_child( n1,  ik_guid(2));

    ik_effector *e1, *e2;
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

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(1));

    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    ik_node_data_t* nda = solver->ndv.node_data;
    EXPECT_THAT(nda->node_count, Eq(3));
    EXPECT_THAT(nda->user_data[0], Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(nda->user_data[2], Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(nda->effector[2], Eq(IK_NODE_EFFECTOR(n2)));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, split_trees_on_effectors)
{
    ik_solvers solvers;
    ik_node *tree, *n1, *n2, *n3, *n4, *n5, *n6;

    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(tree, ik_guid(1));
    ik_node* n2 = ik_node_create_child( n1,  ik_guid(2));
    ik_node* n3 = ik_node_create_child( n2,  ik_guid(3));
    ik_node* n4 = ik_node_create_child( n3,  ik_guid(4));
    ik_node* n5 = ik_node_create_child( n4,  ik_guid(5));
    ik_node* n6 = ik_node_create_child( n5,  ik_guid(6));

    ik_effector *e1, *e2, *e3;
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

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(3));

    ik_solver *solver1, *solver2, *solver3;
    solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);
    solver3 = *(ik_solver**)vector_get_element(&solvers.solver_list, 2);

    EXPECT_THAT(solver1->ndv.node_data, Eq(tree->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n1->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n2->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n3->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n4->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n5->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(n6->d));  // Should point to different node data
    EXPECT_THAT(solver1->ndv.node_data->node_count, Eq(6));  // Nodes 0-5

    // Nodes 0,1,2 should be in first node data view
    EXPECT_THAT(solver1->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver1->ndv.chain_end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // Nodes 2,3 should be in the second node data view
    EXPECT_THAT(solver2->ndv.subbase_idx, Eq(2));
    EXPECT_THAT(solver2->ndv.chain_begin_idx, Eq(3));
    EXPECT_THAT(solver2->ndv.chain_end_idx, Eq(4));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n3)));

    // Nodes 3,4,5 should be in the third node data view
    EXPECT_THAT(solver3->ndv.subbase_idx, Eq(3));
    EXPECT_THAT(solver3->ndv.chain_begin_idx, Eq(4));
    EXPECT_THAT(solver3->ndv.chain_end_idx, Eq(6));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n3)));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n5)));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, split_tree_can_be_flattened_multiple_times)
{
    ik_node *tree, *n1, *n2, *dead3, *n4, *n5, *dead6, *dead7, *dead8, *dead9, *dead10, *dead11, *dead12, *dead13;

    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(    tree,  ik_guid(1));
    ik_node* n2 = ik_node_create_child(    n1,    ik_guid(2));
    ik_node* dead3 = ik_node_create_child( n2,    ik_guid(3));
    ik_node* n4 = ik_node_create_child(    dead3, ik_guid(4));
    ik_node* n5 = ik_node_create_child(    n4,    ik_guid(5));
    ik_node* dead6 = ik_node_create_child( n5,    ik_guid(6));
    ik_node* dead7 = ik_node_create_child( tree,  ik_guid(7));
    ik_node* dead8 = ik_node_create_child( n1,    ik_guid(8));
    ik_node* dead9 = ik_node_create_child( n2,    ik_guid(9));
    ik_node* dead10 = ik_node_create_child(dead3, ik_guid(10));
    ik_node* dead11 = ik_node_create_child(n4,    ik_guid(11));
    ik_node* dead12 = ik_node_create_child(n5,    ik_guid(12));
    ik_node* dead13 = ik_node_create_child(dead6, ik_guid(13));

    ik_effector *e1, *e2;
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

    ik_solvers solvers1;
    ik_solvers_init(&solvers1);
    ASSERT_THAT(ik_solvers_update(&solvers1, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers1.solver_list), Eq(2));
    ik_solver* j1solver1 = *(ik_solver**)vector_get_element(&solvers1.solver_list, 0);
    ik_solver* j1solver2 = *(ik_solver**)vector_get_element(&solvers1.solver_list, 1);
    EXPECT_THAT(j1solver1->ndv.node_data->refcount->refs, Eq(7));  // nodes 0,1,2,4,5 and solvers's solver1->ndv,solver2->ndv are holding a ref

    ik_solvers solvers2;
    ik_solvers_init(&solvers2);
    ASSERT_THAT(ik_solvers_update(&solvers2, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers2.solver_list), Eq(2));
    ik_solver* j2solver1 = *(ik_solver**)vector_get_element(&solvers2.solver_list, 0);
    ik_solver* j2solver2 = *(ik_solver**)vector_get_element(&solvers2.solver_list, 1);
    EXPECT_THAT(j2solver1->ndv.node_data->refcount->refs, Eq(7));  // nodes 0,1,2,4,5 and solvers's solver1->ndv,solver2->ndv are holding a ref

    ik_solvers solvers3;
    ik_solvers_init(&solvers3);
    ASSERT_THAT(ik_solvers_update(&solvers3, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers3.solver_list), Eq(2));
    ik_solver* j3solver1 = *(ik_solver**)vector_get_element(&solvers3.solver_list, 0);
    ik_solver* j3solver2 = *(ik_solver**)vector_get_element(&solvers3.solver_list, 1);
    EXPECT_THAT(j3solver1->ndv.node_data->refcount->refs, Eq(7));  // nodes 0,1,2,4,5 and solvers's solver1->ndv,solver2->ndv are holding a ref

    // The newly created flattened node data should be the one pointing to
    // the original tree node data
    EXPECT_THAT(j1solver1->ndv.node_data, Ne(tree->d));
    EXPECT_THAT(j2solver1->ndv.node_data, Ne(tree->d));
    EXPECT_THAT(j3solver1->ndv.node_data, Eq(tree->d));
    EXPECT_THAT(j1solver2->ndv.node_data, Ne(tree->d));
    EXPECT_THAT(j2solver2->ndv.node_data, Ne(tree->d));
    EXPECT_THAT(j3solver2->ndv.node_data, Eq(tree->d));

    EXPECT_THAT(j1solver1->ndv.node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    EXPECT_THAT(j2solver1->ndv.node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    EXPECT_THAT(j3solver1->ndv.node_data->refcount->refs, Eq(7));  // nodes 0,1,2,4,5 and nvd1,nvd2 are holding refs
    ik_node_free_recursive(tree);
    EXPECT_THAT(j1solver1->ndv.node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    EXPECT_THAT(j2solver1->ndv.node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    EXPECT_THAT(j3solver1->ndv.node_data->refcount->refs, Eq(2));  // nvd1,nvd2 are holding refs
    ik_solvers_deinit(&solvers1);
    EXPECT_THAT(j2solver1->ndv.node_data->refcount->refs, Eq(2));
    EXPECT_THAT(j3solver1->ndv.node_data->refcount->refs, Eq(2));
    ik_solvers_deinit(&solvers2);
    EXPECT_THAT(j3solver1->ndv.node_data->refcount->refs, Eq(2));
    ik_solvers_deinit(&solvers3);
}

TEST_F(NAME, split_trees_on_effectors_with_chain_lengths)
{
    ik_solvers solvers;
    ik_node *tree, *n1, *n2, *n3, *n4, *n5, *n6;

    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(tree, ik_guid(1));
    ik_node* n2 = ik_node_create_child( n1,  ik_guid(2));
    ik_node* n3 = ik_node_create_child( n2,  ik_guid(3));
    ik_node* n4 = ik_node_create_child( n3,  ik_guid(4));
    ik_node* n5 = ik_node_create_child( n4,  ik_guid(5));
    ik_node* n6 = ik_node_create_child( n5,  ik_guid(6));

    ik_effector *e1, *e2, *e3;
    ik_node_create_effector(&e1, n2);
    ik_node_create_effector(&e2, n3);
    ik_node_create_effector(&e3, n5);
    e3->chain_length = 1;

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

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(3));

    ik_solver *solver1, *solver2, *solver3;
    solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);
    solver3 = *(ik_solver**)vector_get_element(&solvers.solver_list, 2);

    EXPECT_THAT(solver1->ndv.node_data, Eq(tree->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n1->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n2->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n3->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n4->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n5->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(n6->d));  // Should point to different node data
    EXPECT_THAT(solver1->ndv.node_data->node_count, Eq(6));  // Nodes 0-5

    // Nodes 0,1,2 should be in first node data view
    EXPECT_THAT(solver1->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver1->ndv.chain_end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // Nodes 2,3 should be in the second node data view
    EXPECT_THAT(solver2->ndv.subbase_idx, Eq(2));
    EXPECT_THAT(solver2->ndv.chain_begin_idx, Eq(3));
    EXPECT_THAT(solver2->ndv.chain_end_idx, Eq(4));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n3)));

    // Nodes 4,5 should be in the third node data view
    EXPECT_THAT(solver3->ndv.subbase_idx, Eq(4));
    EXPECT_THAT(solver3->ndv.chain_begin_idx, Eq(5));
    EXPECT_THAT(solver3->ndv.chain_end_idx, Eq(6));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n5)));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, ignore_effector_on_root_node_with_dead_nodes)
{
    ik_solvers solvers;
    ik_node *tree, *n1, *n2, *n3, *n4;

    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(tree, ik_guid(1));
    ik_node* n2 = ik_node_create_child(n1,  ik_guid(2));
    ik_node* n3 = ik_node_create_child(tree,  ik_guid(3));
    ik_node* n4 = ik_node_create_child(n1,  ik_guid(4));

    ik_effector *e1, *e2;
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

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(1));

    ik_solver* solver = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    EXPECT_THAT(solver->ndv.node_data->node_count, Eq(3));
    EXPECT_THAT(solver->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver->ndv.chain_end_idx, Eq(3));
    EXPECT_THAT(solver->ndv.node_data, Eq(tree->d));
    EXPECT_THAT(solver->ndv.node_data, Eq(n1->d));
    EXPECT_THAT(solver->ndv.node_data, Eq(n2->d));
    EXPECT_THAT(solver->ndv.node_data, Ne(n3->d));
    EXPECT_THAT(solver->ndv.node_data, Ne(n4->d));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, split_trees_on_effectors_with_dead_nodes)
{
    ik_solvers solvers;
    ik_node *tree, *n1, *n2, *dead3, *n4, *n5, *dead6, *dead7, *dead8, *dead9, *dead10, *dead11, *dead12, *dead13;

    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(    tree,  ik_guid(1));
    ik_node* n2 = ik_node_create_child(    n1,    ik_guid(2));
    ik_node* dead3 = ik_node_create_child( n2,    ik_guid(3));
    ik_node* n4 = ik_node_create_child(    dead3, ik_guid(4));
    ik_node* n5 = ik_node_create_child(    n4,    ik_guid(5));
    ik_node* dead6 = ik_node_create_child( n5,    ik_guid(6));
    ik_node* dead7 = ik_node_create_child( tree,  ik_guid(7));
    ik_node* dead8 = ik_node_create_child( n1,    ik_guid(8));
    ik_node* dead9 = ik_node_create_child( n2,    ik_guid(9));
    ik_node* dead10 = ik_node_create_child(dead3, ik_guid(10));
    ik_node* dead11 = ik_node_create_child(n4,    ik_guid(11));
    ik_node* dead12 = ik_node_create_child(n5,    ik_guid(12));
    ik_node* dead13 = ik_node_create_child(dead6, ik_guid(13));

    ik_effector *e1, *e2;
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

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(2));

    ik_solver *solver1, *solver2;
    solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);

    // Make sure node data was created properly
    EXPECT_THAT(solver1->ndv.node_data, Eq(tree->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n1->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n2->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead3->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n4->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n5->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead6->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead7->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead8->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead9->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead10->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead11->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead12->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead13->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(solver2->ndv.node_data));
    EXPECT_THAT(solver1->ndv.node_data->node_count, Eq(5));

    // Nodes 0,1,2 should be in first node data view
    EXPECT_THAT(solver1->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver1->ndv.chain_end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // Nodes 4,5 should be in the second node data view
    EXPECT_THAT(solver2->ndv.subbase_idx, Eq(3));
    EXPECT_THAT(solver2->ndv.chain_begin_idx, Eq(4));
    EXPECT_THAT(solver2->ndv.chain_end_idx, Eq(5));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n5)));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, split_trees_on_effectors_with_chain_lengths_with_dead_nodes)
{
    ik_solvers solvers;
    ik_node *tree, *n1, *n2, *n3, *n4, *n5, *dead6, *dead7, *dead8, *dead9, *dead10, *dead11, *dead12, *dead13;

    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(tree, ik_guid(1));
    ik_node* n2 = ik_node_create_child(n1,  ik_guid(2));
    ik_node* n3 = ik_node_create_child(n2,  ik_guid(3));
    ik_node* n4 = ik_node_create_child(n3,  ik_guid(4));
    ik_node* n5 = ik_node_create_child(n4,  ik_guid(5));
    ik_node* dead6 = ik_node_create_child(n5,  ik_guid(6));
    ik_node* dead7 = ik_node_create_child(tree,  ik_guid(7));
    ik_node* dead8 = ik_node_create_child(n1,  ik_guid(8));
    ik_node* dead9 = ik_node_create_child(n2,  ik_guid(9));
    ik_node* dead10 = ik_node_create_child(n3,  ik_guid(10));
    ik_node* dead11 = ik_node_create_child(n4,  ik_guid(11));
    ik_node* dead12 = ik_node_create_child(n5,  ik_guid(12));
    ik_node* dead13 = ik_node_create_child(dead6,  ik_guid(13));

    ik_effector *e1, *e2, *e3;
    ik_node_create_effector(&e1, n2);
    ik_node_create_effector(&e2, n3);
    ik_node_create_effector(&e3, n5);
    e3->chain_length = 1;

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

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(3));

    ik_solver *solver1, *solver2, *solver3;
    solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);
    solver3 = *(ik_solver**)vector_get_element(&solvers.solver_list, 2);

    // Make sure node data was created properly
    EXPECT_THAT(solver1->ndv.node_data, Eq(tree->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n1->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n2->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n3->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n4->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(n5->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead6->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead7->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead8->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead9->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead10->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead11->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead12->d));
    EXPECT_THAT(solver1->ndv.node_data, Ne(dead13->d));
    EXPECT_THAT(solver1->ndv.node_data, Eq(solver2->ndv.node_data));
    EXPECT_THAT(solver1->ndv.node_data, Eq(solver3->ndv.node_data));
    EXPECT_THAT(solver1->ndv.node_data->node_count, Eq(6));

    // Nodes 0,1,2 should be in first node data view
    EXPECT_THAT(solver1->ndv.subbase_idx, Eq(0));
    EXPECT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    EXPECT_THAT(solver1->ndv.chain_end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // Nodes 2,3 should be in the second node data view
    EXPECT_THAT(solver2->ndv.subbase_idx, Eq(2));
    EXPECT_THAT(solver2->ndv.chain_begin_idx, Eq(3));
    EXPECT_THAT(solver2->ndv.chain_end_idx, Eq(4));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n3)));

    // Nodes 4,5 should be in the third node data view
    EXPECT_THAT(solver3->ndv.subbase_idx, Eq(4));
    EXPECT_THAT(solver3->ndv.chain_begin_idx, Eq(5));
    EXPECT_THAT(solver3->ndv.chain_end_idx, Eq(6));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n5)));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, split_trees_with_dead_nodes)
{
    ik_node *tree, *n1, *n2, *n3, *n4, *n5, *n6;
    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(tree, ik_guid(1));
    ik_node* n2 = ik_node_create_child(n1, ik_guid(2));
    ik_node* n3 = ik_node_create_child(n2, ik_guid(3));
    ik_node* n4 = ik_node_create_child(n3, ik_guid(4));
    ik_node* n5 = ik_node_create_child(n4, ik_guid(5));
    ik_node* n6 = ik_node_create_child(n5, ik_guid(6));

    ik_effector *e1, *e2;
    ik_effector_create(&e1);
    ik_effector_create(&e2);
    ik_node_attach_effector(n3, e1);
    ik_node_attach_effector(n6, e2);
    e2->chain_length = 1;

    ik_solvers solvers;
    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(2));

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

    struct ik_solver* solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    struct ik_solver* solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);
    ASSERT_THAT(solver1->ndv.node_data->node_count, Eq(6));  // excludes node 4

    EXPECT_THAT(solver1->ndv.subbase_idx, Eq(0));
    ASSERT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    ASSERT_THAT(solver1->ndv.chain_end_idx, Eq(4));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 3), Eq(IK_NODE_USER_DATA(n3)));

    EXPECT_THAT(solver2->ndv.subbase_idx, Eq(4));
    ASSERT_THAT(solver2->ndv.chain_begin_idx, Eq(5));
    ASSERT_THAT(solver2->ndv.chain_end_idx, Eq(6));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n5)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n6)));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, split_tree_with_effector_at_junction)
{
    ik_solvers solvers;
    ik_node* tree = tree_with_two_effectors();
    ik_node* n1 = ik_node_find(tree, ik_guid(1));
    ik_node* n2 = ik_node_find(tree, ik_guid(2));
    ik_node* n3 = ik_node_find(tree, ik_guid(3));
    ik_node* n4 = ik_node_find(tree, ik_guid(4));
    ik_node* n5 = ik_node_find(tree, ik_guid(5));
    ik_node* n6 = ik_node_find(tree, ik_guid(6));
    ik_node* n7 = ik_node_find(tree, ik_guid(7));
    ik_node* n8 = ik_node_find(tree, ik_guid(8));
    ik_node* n9 = ik_node_find(tree, ik_guid(9));

    ik_effector* e3;
    ik_node_create_effector(&e3, n3);

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));

    //
    // e1 -> 6           9 <- e2
    //        \         /
    //         5       8
    //          \     /
    //           4   7
    //            \ /
    //             3 <- e3
    //             |
    //             2
    //             |
    //             1
    //             |
    //             0 <- a1
    //

    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(3));
    ik_solver* solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    ik_solver* solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);
    ik_solver* solver3 = *(ik_solver**)vector_get_element(&solvers.solver_list, 2);
    ASSERT_THAT(solver1->ndv.node_data->node_count, Eq(10));

    ASSERT_THAT(solver1->ndv.subbase_idx, Eq(0));
    ASSERT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    ASSERT_THAT(solver1->ndv.chain_end_idx, Eq(4));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 3), Eq(IK_NODE_USER_DATA(n3)));

    ASSERT_THAT(solver2->ndv.subbase_idx, Eq(3));
    ASSERT_THAT(solver2->ndv.chain_begin_idx, Eq(4));
    ASSERT_THAT(solver2->ndv.chain_end_idx, Eq(7));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n3)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n5)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 3), Eq(IK_NODE_USER_DATA(n6)));

    ASSERT_THAT(solver3->ndv.subbase_idx, Eq(3));
    ASSERT_THAT(solver3->ndv.chain_end_idx, Eq(7));
    ASSERT_THAT(solver3->ndv.chain_end_idx, Eq(9));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n3)));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n5)));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 3), Eq(IK_NODE_USER_DATA(n6)));

    ik_solvers_deinit(&solvers);
    ik_node_free_recursive(tree);
}

TEST_F(NAME, dd)
{
    //
    //                e2       e3
    //                |         |
    //                v         v
    //   e1 -> 9     12         18     21 <- e4
    //          \    |           |    /
    //           8   11         17   20
    //            \  |           |  /
    //             7 10         16 19
    //              \|           |/
    //               6           15
    //                \         /
    //                 5       14
    //                  \     /
    //                   4   13
    //                    \ /
    //                     3
    //                     |
    //                     2
    //                     |
    //                     1
    //                     |
    //                     0
    //
}

TEST_F(NAME, check_solvers_order_for_disjoint_trees_llr)
{
    ik_solvers solvers;
    ik_node* tree = tree_llr();
    ik_node *e1, *e2, *e3;

    // Need to change effector chain lengths so tree becomes disjoint
    e1 = ik_node_find(tree, ik_guid(6));
    e2 = ik_node_find(tree, ik_guid(8));
    e3 = ik_node_find(tree, ik_guid(10));
    IK_NODE_EFFECTOR(e1)->chain_length = 1;
    IK_NODE_EFFECTOR(e2)->chain_length = 1;

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(3));

    //
    // The scenario here is nodes 0,1,2,9,10 form a chain (call this
    // "ndv 1"), nodes 5,6 form a chain ("ndv 2"), and nodes 7,8 form
    // a chain ("ndv 3"). Because nd's 2 and 3 depend on the solution of
    // nd 1, nd 1 must appear before ndv's 2 and 3 in the solvers.
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
    struct ik_node *b1, *b2, *b3;
    b1 = tree;
    b2 = ik_node_find(tree, ik_guid(5));  // expected base of second nd
    b3 = ik_node_find(tree, ik_guid(7));  // expected base of third nd
    struct ik_solver* solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    struct ik_solver* solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);
    struct ik_solver* solver3 = *(ik_solver**)vector_get_element(&solvers.solver_list, 2);

    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(b1)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(b2)));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(b3)));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}

TEST_F(NAME, check_solvers_order_for_disjoint_trees_llrr)
{
    ik_solvers solvers;
    ik_node *tree, *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9, *n10, *n11, *n12, *n13, *n14, *n15;

    ik_node* tree = ik_node_create(ik_guid(0));
    ik_node* n1 = ik_node_create_child(tree, ik_guid(1));
    ik_node* n2 = ik_node_create_child( n1,  ik_guid(2));
    ik_node* n3 = ik_node_create_child( n2,  ik_guid(3));
    ik_node* n4 = ik_node_create_child( n3,  ik_guid(4));
    ik_node* n5 = ik_node_create_child( n4,  ik_guid(5));
    ik_node* n6 = ik_node_create_child( n5,  ik_guid(6));
    ik_node* n7 = ik_node_create_child( n6,  ik_guid(7));
    ik_node* n8 = ik_node_create_child( n5,  ik_guid(8));
    ik_node* n9 = ik_node_create_child( n8,  ik_guid(9));
    ik_node* n10 = ik_node_create_child(n2,  ik_guid(10));
    ik_node* n11 = ik_node_create_child(n10, ik_guid(11));
    ik_node* n12 = ik_node_create_child(n11, ik_guid(12));
    ik_node* n13 = ik_node_create_child(n12, ik_guid(13));
    ik_node* n14 = ik_node_create_child(n11, ik_guid(14));
    ik_node* n15 = ik_node_create_child(n14, ik_guid(15));

    ik_effector *e1, *e2, *e3, *e4, *e5;
    ik_node_create_effector(&e1, n7);
    ik_node_create_effector(&e2, n9);
    ik_node_create_effector(&e3, n13);
    ik_node_create_effector(&e4, n15);
    ik_node_create_effector(&e5, n2);
    e1->chain_length = 3;
    e2->chain_length = 1;
    e3->chain_length = 1;
    e4->chain_length = 4;

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

    ik_solvers_init(&solvers);
    ASSERT_THAT(ik_solvers_update(&solvers, tree), Eq(IK_OK));
    ASSERT_THAT(vector_count(&solvers.solver_list), Eq(5));
    struct ik_solver* solver1 = *(ik_solver**)vector_get_element(&solvers.solver_list, 0);
    struct ik_solver* solver2 = *(ik_solver**)vector_get_element(&solvers.solver_list, 1);
    struct ik_solver* solver3 = *(ik_solver**)vector_get_element(&solvers.solver_list, 2);
    struct ik_solver* solver4 = *(ik_solver**)vector_get_element(&solvers.solver_list, 3);
    struct ik_solver* solver5 = *(ik_solver**)vector_get_element(&solvers.solver_list, 4);

    EXPECT_THAT(solver1->ndv.node_data->node_count, Eq(15));

    // 0,1,2
    EXPECT_THAT(solver1->ndv.subbase_idx, Eq(0));
    ASSERT_THAT(solver1->ndv.chain_begin_idx, Eq(1));
    ASSERT_THAT(solver1->ndv.chain_end_idx, Eq(3));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(tree)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n1)));
    EXPECT_THAT(IK_NDV_AT(&solver1->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n2)));

    // 2,10,11,14,15
    EXPECT_THAT(solver2->ndv.subbase_idx, Eq(2));
    ASSERT_THAT(solver2->ndv.chain_begin_idx, Eq(3));
    ASSERT_THAT(solver2->ndv.chain_end_idx, Eq(7));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n2)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n10)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n11)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 3), Eq(IK_NODE_USER_DATA(n14)));
    EXPECT_THAT(IK_NDV_AT(&solver2->ndv, user_data, 4), Eq(IK_NODE_USER_DATA(n15)));

    // 12,13
    EXPECT_THAT(solver3->ndv.subbase_idx, Eq(7));
    ASSERT_THAT(solver3->ndv.chain_begin_idx, Eq(8));
    ASSERT_THAT(solver3->ndv.chain_end_idx, Eq(9));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n12)));
    EXPECT_THAT(IK_NDV_AT(&solver3->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n13)));

    // 4,5,6,7
    EXPECT_THAT(solver4->ndv.subbase_idx, Eq(9));
    ASSERT_THAT(solver4->ndv.chain_begin_idx, Eq(10));
    ASSERT_THAT(solver4->ndv.chain_end_idx, Eq(13));
    EXPECT_THAT(IK_NDV_AT(&solver4->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n4)));
    EXPECT_THAT(IK_NDV_AT(&solver4->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n5)));
    EXPECT_THAT(IK_NDV_AT(&solver4->ndv, user_data, 2), Eq(IK_NODE_USER_DATA(n6)));
    EXPECT_THAT(IK_NDV_AT(&solver4->ndv, user_data, 3), Eq(IK_NODE_USER_DATA(n7)));

    // 8,9
    EXPECT_THAT(solver5->ndv.subbase_idx, Eq(13));
    ASSERT_THAT(solver5->ndv.chain_begin_idx, Eq(14));
    ASSERT_THAT(solver5->ndv.chain_end_idx, Eq(15));
    EXPECT_THAT(IK_NDV_AT(&solver5->ndv, user_data, 0), Eq(IK_NODE_USER_DATA(n8)));
    EXPECT_THAT(IK_NDV_AT(&solver5->ndv, user_data, 1), Eq(IK_NODE_USER_DATA(n9)));

    ik_node_free_recursive(tree);
    ik_solvers_deinit(&solvers);
}
*/
