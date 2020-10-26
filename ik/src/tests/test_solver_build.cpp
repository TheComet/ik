#include "gmock/gmock.h"
#include "ik/algorithm.h"
#include "ik/chain_tree.h"
#include "ik/cpputils.hpp"
#include "ik/bone.h"
#include "ik/solver.h"

#define NAME solver_build

using namespace ::testing;

struct ik_solver_dummy
{
    IK_SOLVER_HEAD

    struct ik_chain chain_tree;
};

struct ik_solver_group
{
    IK_SOLVER_HEAD

    struct ik_solver** subsolvers;
    int subsolver_count;
};

static int dummy_init(struct ik_solver* solver_base, const struct ik_subtree* subtree) {
    struct ik_solver_dummy* solver = (struct ik_solver_dummy*)solver_base;
    chain_tree_init(&solver->chain_tree);
    return chain_tree_build(&solver->chain_tree, subtree);
}
static void dummy_deinit(struct ik_solver* solver_base) {
    struct ik_solver_dummy* solver = (struct ik_solver_dummy*)solver_base;
    chain_tree_deinit(&solver->chain_tree);
}
static int  dummy_solve(struct ik_solver* solver_base) { return 1; }

const struct ik_solver_interface ik_solver_DUMMY1 = {
    "dummy1",
    sizeof(struct ik_solver_dummy),
    dummy_init,
    dummy_deinit,
    dummy_solve
};
const struct ik_solver_interface ik_solver_DUMMY2 = {
    "dummy2",
    sizeof(struct ik_solver_dummy),
    dummy_init,
    dummy_deinit,
    dummy_solve
};
const struct ik_solver_interface ik_solver_DUMMY3 = {
    "dummy3",
    sizeof(struct ik_solver_dummy),
    dummy_init,
    dummy_deinit,
    dummy_solve
};

class NAME : public Test
{
public:
    ik_bone* tree_without_effectors()
    {
        //
        //   .           .
        //    5         8
        //     .       .
        //      4     7
        //       .   .
        //        3 6
        //         .
        //         2
        //         .
        //         1
        //         .
        //         0
        //         .
        //
        ik_bone* tree = ik_bone_create();          tree->user_data = (void*)0;
        ik_bone* b1 = ik_bone_create_child(tree);  b1->user_data = (void*)1;
        ik_bone* b2 = ik_bone_create_child(b1);    b2->user_data = (void*)2;
        ik_bone* b3 = ik_bone_create_child(b2);    b3->user_data = (void*)3;
        ik_bone* b4 = ik_bone_create_child(b3);    b4->user_data = (void*)4;
        ik_bone* b5 = ik_bone_create_child(b4);    b5->user_data = (void*)5;
        ik_bone* b6 = ik_bone_create_child(b2);    b6->user_data = (void*)6;
        ik_bone* b7 = ik_bone_create_child(b6);    b7->user_data = (void*)7;
        ik_bone* b8 = ik_bone_create_child(b7);    b8->user_data = (void*)8;
        return tree;
    }

    ik_bone* tree_with_two_effectors_and_no_algorithms()
    {
        ik_bone *tree, *b5, *b8;
        tree = tree_without_effectors();
        b5 = ik_bone_find(tree, (void*)5);
        b8 = ik_bone_find(tree, (void*)8);

        ik_bone_create_effector(b5);
        ik_bone_create_effector(b8);

        return tree;
    }

    ik_bone* tree_with_two_effectors()
    {
        ik_bone* tree = tree_with_two_effectors_and_no_algorithms();
        ik_bone_create_algorithm(tree, "dummy1");
        return tree;
    }

    virtual void SetUp() override
    {
        ik_solver_register(&ik_solver_DUMMY1);
        ik_solver_register(&ik_solver_DUMMY2);
        ik_solver_register(&ik_solver_DUMMY3);
    }

    virtual void TearDown() override
    {
        ik_solver_unregister(&ik_solver_DUMMY3);
        ik_solver_unregister(&ik_solver_DUMMY2);
        ik_solver_unregister(&ik_solver_DUMMY1);
    }
};

TEST_F(NAME, no_action_if_tree_has_no_effectors_or_algorithms)
{
    ik::Ref<ik_bone> tree = tree_without_effectors();
    ik::Ref<ik_solver> solver = ik_solver_build(tree);
    EXPECT_THAT(solver.isNull(), IsTrue());
}

TEST_F(NAME, no_action_if_tree_has_no_algorithms)
{
    ik::Ref<ik_bone> tree = tree_without_effectors();
    ik::Ref<ik_bone> b5 = ik_bone_find(tree, (void*)5);
    ik::Ref<ik_bone> b8 = ik_bone_find(tree, (void*)8);

    ik_effector* eff1 = ik_bone_create_effector(b5);
    ik_effector* eff2 = ik_bone_create_effector(b8);

    ik::Ref<ik_solver> solver = ik_solver_build(tree);
    EXPECT_THAT(solver.isNull(), IsTrue());
}

TEST_F(NAME, check_refcounts_are_correct)
{
    // TODO
    ik::Ref<ik_bone> tree = tree_with_two_effectors();
    ik::Ref<ik_solver> solver = ik_solver_build(tree);
    ASSERT_THAT(solver.isNull(), IsFalse());
    ASSERT_THAT(solver->algorithm, NotNull());
    EXPECT_THAT(solver->algorithm->type, StrEq("dummy1"));

    EXPECT_THAT(solver->refcount->refs, Eq(1));
}

TEST_F(NAME, always_find_root_most_algorithm)
{
    ik::Ref<ik_bone> tree = tree_with_two_effectors_and_no_algorithms();
    ik::Ref<ik_bone> b2 = ik_bone_find(tree, (void*)2);

    ik_algorithm* a1 = ik_bone_create_algorithm(tree, "dummy1");
    ik_algorithm* a2 = ik_bone_create_algorithm(b2, "dummy2");

    ik::Ref<ik_solver> solver = ik_solver_build(tree);

    //
    //       .           .
    //  e1 -> 5         8 <- e2
    //         .       .
    //          4     7
    //           .   .
    //            3 6
    //             .
    //             2 <- a2
    //             .
    //             1
    //             .
    //             0 <- a1
    //             .
    //

    ASSERT_THAT(solver.isNull(), IsFalse());
    ASSERT_THAT(solver->algorithm, NotNull());
    EXPECT_THAT(solver->algorithm, Eq(a1));
}

TEST_F(NAME, choose_next_available_algorithm_after_chain_ends_exact)
{
    ik::Ref<ik_bone> tree = tree_with_two_effectors_and_no_algorithms();

    ik::Ref<ik_bone> b2 = ik_bone_find(tree, (void*)2);
    ik::Ref<ik_bone> b5 = ik_bone_find(tree, (void*)5);
    ik::Ref<ik_bone> b8 = ik_bone_find(tree, (void*)8);

    ik_algorithm* a1 = ik_bone_create_algorithm(tree, "dummy1");
    ik_algorithm* a2 = ik_bone_create_algorithm(b2, "dummy2");

    b5->effector->chain_length = 3;
    b8->effector->chain_length = 3;

    ik::Ref<ik_solver> solver = ik_solver_build(tree);

    //
    //       .           .
    //  e1 -> 5         8 <- e2
    //         .       .
    //          4     7
    //           .   .
    //            3 6
    //             . <- chains end here
    //             2 <- a2
    //             .
    //             1
    //             .
    //             0 <- a1
    //             .
    //

    ASSERT_THAT(solver.isNull(), IsFalse());
    ASSERT_THAT(solver->impl.name, StrEq("group"));
    ASSERT_THAT(((ik_solver_group*)solver.get())->subsolver_count, Eq(2));

    // We expect algorithm 2 to be chosen because it is the next available
    // one after the chains end
    ik::Ref<ik_solver> s1 = ((ik_solver_group*)solver.get())->subsolvers[0];
    ASSERT_THAT(s1->algorithm, NotNull());
    EXPECT_THAT(s1->algorithm, Eq(a2));
    ik::Ref<ik_solver> s2 = ((ik_solver_group*)solver.get())->subsolvers[1];
    ASSERT_THAT(s2->algorithm, NotNull());
    EXPECT_THAT(s2->algorithm, Eq(a2));
}

TEST_F(NAME, choose_next_available_algorithm_after_chain_ends_1)
{
    ik::Ref<ik_bone> tree = tree_with_two_effectors_and_no_algorithms();

    ik::Ref<ik_bone> b4 = ik_bone_find(tree, (void*)4);
    ik::Ref<ik_bone> b5 = ik_bone_find(tree, (void*)5);
    ik::Ref<ik_bone> b8 = ik_bone_find(tree, (void*)8);

    ik_algorithm* a1 = ik_bone_create_algorithm(tree, "dummy1");
    ik_algorithm* a2 = ik_bone_create_algorithm(b4, "dummy2");

    b5->effector->chain_length = 2;
    b8->effector->chain_length = 2;

    ik::Ref<ik_solver> solver = ik_solver_build(tree);

    //
    //       .           .
    //  e1 -> 5         8 <- e2
    //         .       .
    //    a2 -> 4     7
    //           .   . <- chains end here
    //            3 6
    //             .
    //             2
    //             .
    //             1
    //             .
    //             0 <- a1
    //             .
    //

    ASSERT_THAT(solver.isNull(), IsFalse());
    ASSERT_THAT(solver->impl.name, StrEq("group"));
    ASSERT_THAT(((ik_solver_group*)solver.get())->subsolver_count, Eq(2));

    // We expect algorithm 1 to be chosen because it is the next available
    // one after the chain's end
    ik::Ref<ik_solver> s1 = ((ik_solver_group*)solver.get())->subsolvers[0];
    ASSERT_THAT(s1->impl.name, StrEq("dummy1"));
    ASSERT_THAT(s1->algorithm, NotNull());
    EXPECT_THAT(s1->algorithm, Eq(a1));

    // Other solver's chain ends on algorithm 2 so it should be chosen
    ik::Ref<ik_solver> s2 = ((ik_solver_group*)solver.get())->subsolvers[1];
    ASSERT_THAT(s2->impl.name, StrEq("dummy2"));
    ASSERT_THAT(s2->algorithm, NotNull());
    EXPECT_THAT(s2->algorithm, Eq(a2));
}

TEST_F(NAME, choose_next_available_algorithm_after_chain_ends_2)
{
    ik::Ref<ik_bone> tree = tree_with_two_effectors_and_no_algorithms();

    ik::Ref<ik_bone> b4 = ik_bone_find(tree, (void*)4);
    ik::Ref<ik_bone> b5 = ik_bone_find(tree, (void*)5);
    ik::Ref<ik_bone> b8 = ik_bone_find(tree, (void*)8);

    ik_algorithm* a1 = ik_bone_create_algorithm(tree, "dummy1");
    ik_algorithm* a2 = ik_bone_create_algorithm(b4, "dummy2");

    b5->effector->chain_length = 3;
    b8->effector->chain_length = 2;

    ik::Ref<ik_solver> solver = ik_solver_build(tree);

    //
    //       .           .
    //  e1 -> 5         8 <- e2
    //         .       .
    //    a2 -> 4     7
    //           .   . <- e2 chain ends here
    //            3 6
    //             . <- e1 chain ends here
    //             2
    //             .
    //             1
    //             .
    //             0 <- a1
    //             .
    //

    ASSERT_THAT(solver.isNull(), IsFalse());
    ASSERT_THAT(solver->impl.name, StrEq("group"));
    ASSERT_THAT(((ik_solver_group*)solver.get())->subsolver_count, Eq(2));

    // We expect algorithm 1 to be chosen because it is the next available
    // one after the chain's end
    ik::Ref<ik_solver> s1 = ((ik_solver_group*)solver.get())->subsolvers[0];
    ASSERT_THAT(s1->impl.name, StrEq("dummy1"));
    ASSERT_THAT(s1->algorithm, NotNull());
    EXPECT_THAT(s1->algorithm, Eq(a1));

    // Other solver's chain ends on algorithm 2 so it should be chosen
    ik::Ref<ik_solver> s2 = ((ik_solver_group*)solver.get())->subsolvers[1];
    ASSERT_THAT(s2->impl.name, StrEq("dummy1"));
    ASSERT_THAT(s2->algorithm, NotNull());
    EXPECT_THAT(s2->algorithm, Eq(a1));
}

TEST_F(NAME, split_trees_on_effectors)
{
    ik::Ref<ik_bone> tree = ik_bone_create();
    ik::Ref<ik_bone> b1 = ik_bone_create_child(tree);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b3);
    ik::Ref<ik_bone> b5 = ik_bone_create_child(b4);

    ik_effector* e1 = ik_bone_create_effector(b1);
    ik_effector* e2 = ik_bone_create_effector(b2);
    ik_effector* e3 = ik_bone_create_effector(b4);
    ik_algorithm* a = ik_bone_create_algorithm(tree, "dummy1");
    ik::Ref<ik_solver> solver = ik_solver_build(tree);

    //
    //       .
    //       5
    //       .
    //       4 <- e3
    //       .
    //       3
    //       .
    //       2 <- e2
    //       .
    //       1 <- e1
    //       .
    //       0 <- a
    //       .
    //
    ASSERT_THAT(solver.isNull(), IsFalse());
    ASSERT_THAT(solver->impl.name, StrEq("group"));
    ASSERT_THAT(((ik_solver_group*)solver.get())->subsolver_count, Eq(3));

    ik_solver_dummy* s1 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)solver.get())->subsolvers[0]);
    ik_solver_dummy* s2 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)solver.get())->subsolvers[1]);
    ik_solver_dummy* s3 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)solver.get())->subsolvers[2]);

    EXPECT_THAT(s1->algorithm, Eq(a));
    EXPECT_THAT(chain_bone_count(&s1->chain_tree), Eq(2));
    EXPECT_THAT(chain_get_base_bone(&s1->chain_tree), Eq(tree));
    EXPECT_THAT(chain_get_tip_bone(&s1->chain_tree), Eq(b1));

    EXPECT_THAT(s2->algorithm, Eq(a));
    EXPECT_THAT(chain_bone_count(&s2->chain_tree), Eq(1));
    EXPECT_THAT(chain_get_base_bone(&s2->chain_tree), Eq(b2));
    EXPECT_THAT(chain_get_tip_bone(&s2->chain_tree), Eq(b2));

    EXPECT_THAT(s3->algorithm, Eq(a));
    EXPECT_THAT(chain_bone_count(&s3->chain_tree), Eq(2));
    EXPECT_THAT(chain_get_base_bone(&s3->chain_tree), Eq(b3));
    EXPECT_THAT(chain_get_tip_bone(&s3->chain_tree), Eq(b4));
}

TEST_F(NAME, split_trees_on_effectors_and_choose_new_algorithm)
{
    ik::Ref<ik_bone> tree = ik_bone_create();
    ik::Ref<ik_bone> b1 = ik_bone_create_child(tree);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b3);
    ik::Ref<ik_bone> b5 = ik_bone_create_child(b4);

    ik_effector* e1 = ik_bone_create_effector(b1);
    ik_effector* e2 = ik_bone_create_effector(b2);
    ik_effector* e3 = ik_bone_create_effector(b4);

    ik_algorithm* a1 = ik_bone_create_algorithm(b3, "dummy1");
    ik_algorithm* a2 = ik_bone_create_algorithm(b2, "dummy2");
    ik_algorithm* a3 = ik_bone_create_algorithm(tree, "dummy3");

    ik::Ref<ik_solver> solver = ik_solver_build(tree);

    //
    //       .
    //       5
    //       .
    //       4 <- e3
    //       .
    //       3 <- a1
    //       .
    //       2 <- e2, a2
    //       .
    //       1 <- e1
    //       .
    //       0 <- a3
    //       .
    //
    ASSERT_THAT(solver.isNull(), IsFalse());
    ASSERT_THAT(solver->impl.name, StrEq("group"));
    ASSERT_THAT(((ik_solver_group*)solver.get())->subsolver_count, Eq(3));

    ik_solver_dummy* s1 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)solver.get())->subsolvers[0]);
    ik_solver_dummy* s2 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)solver.get())->subsolvers[1]);
    ik_solver_dummy* s3 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)solver.get())->subsolvers[2]);

    EXPECT_THAT(s1->algorithm, Eq(a3));
    EXPECT_THAT(chain_bone_count(&s1->chain_tree), Eq(2));
    EXPECT_THAT(chain_get_base_bone(&s1->chain_tree), Eq(tree));
    EXPECT_THAT(chain_get_tip_bone(&s1->chain_tree), Eq(b1));

    EXPECT_THAT(s2->algorithm, Eq(a2));
    EXPECT_THAT(chain_bone_count(&s2->chain_tree), Eq(1));
    EXPECT_THAT(chain_get_base_bone(&s2->chain_tree), Eq(b2));
    EXPECT_THAT(chain_get_tip_bone(&s2->chain_tree), Eq(b2));

    EXPECT_THAT(s3->algorithm, Eq(a1));
    EXPECT_THAT(chain_bone_count(&s3->chain_tree), Eq(2));
    EXPECT_THAT(chain_get_base_bone(&s3->chain_tree), Eq(b3));
    EXPECT_THAT(chain_get_tip_bone(&s3->chain_tree), Eq(b4));
}

TEST_F(NAME, missing_root_algorithm)
{
    ik::Ref<ik_bone> tree = ik_bone_create();
    ik::Ref<ik_bone> b1 = ik_bone_create_child(tree);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b3);
    ik::Ref<ik_bone> b5 = ik_bone_create_child(b4);

    ik_effector* e1 = ik_bone_create_effector(b1);
    ik_effector* e2 = ik_bone_create_effector(b2);
    ik_effector* e3 = ik_bone_create_effector(b4);

    ik_algorithm* a1 = ik_bone_create_algorithm(b3, "dummy1");
    ik_algorithm* a2 = ik_bone_create_algorithm(b2, "dummy2");

    ik::Ref<ik_solver> solver = ik_solver_build(tree);

    //
    //       .
    //       5
    //       .
    //       4 <- e3
    //       .
    //       3 <- a1
    //       .
    //       2 <- e2, a2
    //       .
    //       1 <- e1
    //       .
    //       0
    //       .
    //
    ASSERT_THAT(solver.isNull(), IsFalse());
    ASSERT_THAT(solver->impl.name, StrEq("group"));
    ASSERT_THAT(((ik_solver_group*)solver.get())->subsolver_count, Eq(2));

    ik_solver_dummy* s1 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)solver.get())->subsolvers[0]);
    ik_solver_dummy* s2 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)solver.get())->subsolvers[1]);

    EXPECT_THAT(s1->algorithm, Eq(a2));
    EXPECT_THAT(chain_bone_count(&s1->chain_tree), Eq(1));
    EXPECT_THAT(chain_get_base_bone(&s1->chain_tree), Eq(b2));
    EXPECT_THAT(chain_get_tip_bone(&s1->chain_tree), Eq(b2));

    EXPECT_THAT(s2->algorithm, Eq(a1));
    EXPECT_THAT(chain_bone_count(&s2->chain_tree), Eq(2));
    EXPECT_THAT(chain_get_base_bone(&s2->chain_tree), Eq(b3));
    EXPECT_THAT(chain_get_tip_bone(&s2->chain_tree), Eq(b4));
}

TEST_F(NAME, ignore_parents_of_root_bone)
{
    ik::Ref<ik_bone> tree = ik_bone_create();
    ik::Ref<ik_bone> b1 = ik_bone_create_child(tree);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2);

    ik_effector* e = ik_bone_create_effector(b3);
    ik_algorithm* a = ik_bone_create_algorithm(tree, "dummy1");

    ik::Ref<ik_solver> s1 = ik_solver_build(tree);
    ik::Ref<ik_solver> s2 = ik_solver_build(b1);
    ik::Ref<ik_solver> s3 = ik_solver_build(b2);

    //
    //      .
    //      2 <- e
    //      .
    //      1
    //      .
    //      0 <- a
    //      .
    //
    ASSERT_THAT(s1.isNull(), IsFalse());
    ASSERT_THAT(s1->impl.name, StrEq("dummy1"));
    ASSERT_THAT(s2.isNull(), IsTrue());
    ASSERT_THAT(s3.isNull(), IsTrue());
}

TEST_F(NAME, chain_ending_in_middle_of_second_chain_creates_two_solvers)
{
    ik::Ref<ik_bone> tree = ik_bone_create();
    ik::Ref<ik_bone> b1 = ik_bone_create_child(tree);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(b2);

    ik_effector* e1 = ik_bone_create_effector(b2);
    ik_effector* e2 = ik_bone_create_effector(b3);
    e2->chain_length = 1;
    ik_algorithm* a = ik_bone_create_algorithm(tree, "dummy1");

    ik::Ref<ik_solver> s = ik_solver_build(tree);

    //
    //   .
    //    2 <- e1
    //     .   .
    //      1 3 <- e2 (chain_length=1)
    //       .
    //       0 <- a
    //       .
    //
    ASSERT_THAT(s.isNull(), IsFalse());
    ASSERT_THAT(s->impl.name, StrEq("group"));
    ASSERT_THAT(((ik_solver_group*)s.get())->subsolver_count, Eq(2));

    ik_solver_dummy* s1 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)s.get())->subsolvers[0]);
    ik_solver_dummy* s2 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)s.get())->subsolvers[1]);

    EXPECT_THAT(s1->algorithm, Eq(a));
    EXPECT_THAT(chain_bone_count(&s1->chain_tree), Eq(3));
    EXPECT_THAT(chain_get_base_bone(&s1->chain_tree), Eq(tree));
    EXPECT_THAT(chain_get_tip_bone(&s1->chain_tree), Eq(b2));
    EXPECT_THAT(chain_child_count(&s1->chain_tree), Eq(0));

    EXPECT_THAT(s2->algorithm, Eq(a));
    EXPECT_THAT(chain_bone_count(&s2->chain_tree), Eq(1));
    EXPECT_THAT(chain_get_base_bone(&s2->chain_tree), Eq(b3));
    EXPECT_THAT(chain_get_tip_bone(&s2->chain_tree), Eq(b3));
    EXPECT_THAT(chain_child_count(&s2->chain_tree), Eq(0));
}

TEST_F(NAME, split_trees_with_dead_bones)
{
    ik::Ref<ik_bone> tree = ik_bone_create();
    ik::Ref<ik_bone> b1 = ik_bone_create_child(tree);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1);
    ik::Ref<ik_bone> b3 = ik_bone_create_child(tree);
    ik::Ref<ik_bone> b4 = ik_bone_create_child(b3);
    ik::Ref<ik_bone> b5 = ik_bone_create_child(b4);

    ik_effector* e1 = ik_bone_create_effector(b2);
    ik_effector* e2 = ik_bone_create_effector(b5);
    e2->chain_length = 1;

    ik_algorithm* a = ik_bone_create_algorithm(tree, "dummy1");

    ik::Ref<ik_solver> s = ik_solver_build(tree);

    //
    //                 .
    //                5 <- e2, chain=1
    //       .       .
    // e1 ->  2     4 <- dead
    //         .   .
    //          1 3
    //           .
    //           0
    //           .
    //
    ASSERT_THAT(s.isNull(), IsFalse());
    ASSERT_THAT(s->impl.name, StrEq("group"));
    ASSERT_THAT(((ik_solver_group*)s.get())->subsolver_count, Eq(2));

    ik_solver_dummy* s1 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)s.get())->subsolvers[0]);
    ik_solver_dummy* s2 = reinterpret_cast<ik_solver_dummy*>(((ik_solver_group*)s.get())->subsolvers[1]);

    EXPECT_THAT(s1->algorithm, Eq(a));
    EXPECT_THAT(chain_bone_count(&s1->chain_tree), Eq(3));
    EXPECT_THAT(chain_get_base_bone(&s1->chain_tree), Eq(tree));
    EXPECT_THAT(chain_get_tip_bone(&s1->chain_tree), Eq(b2));
    EXPECT_THAT(chain_child_count(&s1->chain_tree), Eq(0));

    EXPECT_THAT(s2->algorithm, Eq(a));
    EXPECT_THAT(chain_bone_count(&s2->chain_tree), Eq(1));
    EXPECT_THAT(chain_get_base_bone(&s2->chain_tree), Eq(b5));
    EXPECT_THAT(chain_get_tip_bone(&s2->chain_tree), Eq(b5));
    EXPECT_THAT(chain_child_count(&s2->chain_tree), Eq(0));
}
