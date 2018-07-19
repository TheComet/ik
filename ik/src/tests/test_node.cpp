#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME node

using namespace ::testing;

class NAME : public Test
{
public:
    virtual void SetUp() override
    {
        solver_ = IKAPI.solver.create(IK_FABRIK);
    }

    virtual void TearDown() override
    {
        IKAPI.solver.destroy(solver_);
    }

protected:
    ik_solver_t* solver_;
};

TEST_F(NAME, create_sets_proper_guid)
{
    ik_node_t* n = solver_->node->create(3);
    EXPECT_THAT(n->guid, Eq(3));
    solver_->node->destroy(n);
    n = solver_->node->create(56);
    EXPECT_THAT(n->guid, Eq(56));
    solver_->node->destroy(n);
}

TEST_F(NAME, construct_sets_proper_guid)
{
    ik_node_t n;
    solver_->node->construct(&n, 5);
    EXPECT_THAT(n.guid, Eq(5));
    solver_->node->destruct(&n);
    solver_->node->construct(&n, 54);
    EXPECT_THAT(n.guid, Eq(54));
    solver_->node->destruct(&n);
}

#ifdef DEBUG
TEST_F(NAME, debug_warn_if_node_with_same_guid_is_added)
#else
TEST_F(NAME, ndebug_dont_warn_if_node_with_same_guid_is_added)
#endif
{
    internal::CaptureStdout();
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    ik_node_t* n3 = solver_->node->create_child(n2, 2);
#ifdef DEBUG
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrNe(""));
#else
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrEq(""));
#endif
    solver_->node->destroy(n1);
}

TEST_F(NAME, error_if_child_nodes_have_same_guid)
{
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    ik_node_t* n3 = solver_->node->create(3);
    // Test both add_child and create_child
    EXPECT_THAT(solver_->node->add_child(n1, n3), Eq(IK_HASH_EXISTS));
    EXPECT_THAT(solver_->node->create_child(n1, 3), IsNull());
    solver_->node->destroy(n1);
    solver_->node->destroy(n3);
}

TEST_F(NAME, find_child_includes_root_node)
{
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    ik_node_t* n3 = solver_->node->create_child(n2, 4);
    ik_node_t* find = solver_->node->find_child(n1, 2);
    EXPECT_THAT(find, NotNull());
    solver_->node->destroy(n1);
}

TEST_F(NAME, find_child_from_root)
{
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    ik_node_t* n3 = solver_->node->create_child(n2, 4);
    ik_node_t* n4 = solver_->node->create_child(n2, 5);
    EXPECT_THAT(solver_->node->find_child(n1, 5), NotNull());
    solver_->node->destroy(n1);
}

TEST_F(NAME, find_child_doesnt_find_parents)
{
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    ik_node_t* n3 = solver_->node->create_child(n2, 4);
    ik_node_t* n4 = solver_->node->create_child(n1, 5);
    EXPECT_THAT(solver_->node->find_child(n1, 5), NotNull());
    EXPECT_THAT(solver_->node->find_child(n2, 5), IsNull());
    solver_->node->destroy(n1);
}

TEST_F(NAME, reparent_nodes_works)
{
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    ik_node_t* n3 = solver_->node->create_child(n2, 4);
    ik_node_t* n4 = solver_->node->create_child(n1, 5);
    EXPECT_THAT(solver_->node->child_count(n1), Eq(2u));
    EXPECT_THAT(solver_->node->child_count(n2), Eq(1u));
    solver_->node->add_child(n1, n3);
    EXPECT_THAT(solver_->node->child_count(n1), Eq(3u));
    EXPECT_THAT(solver_->node->child_count(n2), Eq(0u));
    solver_->node->destroy(n1);
}

TEST_F(NAME, unlink_node_works)
{
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    EXPECT_THAT(solver_->node->child_count(n1), Eq(1u));
    solver_->node->unlink(n2);
    EXPECT_THAT(solver_->node->child_count(n1), Eq(0u));
    solver_->node->destroy(n1);
    solver_->node->destroy(n2);
}

TEST_F(NAME, reparent_to_self_causes_death)
{
    ik_node_t* n1 = solver_->node->create(2);
    EXPECT_DEATH(solver_->node->add_child(n1, n1), ".*");
    solver_->node->destroy(n1);
}

TEST_F(NAME, duplicate_works)
{
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    ik_node_t* n3 = solver_->node->create_child(n2, 4);
    ik_node_t* n4 = solver_->node->create_child(n1, 5);

    // Set some params we can test...
    IKAPI.vec3.set(n3->position.f, 1, 2, 3);
    IKAPI.quat.set(n3->rotation.f, 4, 5, 6, 7);
    n3->user_data = this;

    ik_node_t* dup = solver_->node->duplicate(n1, 0);

    ASSERT_THAT(solver_->node->find_child(dup, 2), NotNull());
    ASSERT_THAT(solver_->node->find_child(dup, 3), NotNull());
    ASSERT_THAT(solver_->node->find_child(dup, 4), NotNull());
    ASSERT_THAT(solver_->node->find_child(dup, 5), NotNull());

    EXPECT_THAT(solver_->node->find_child(dup, 4)->position.x, Eq(n3->position.x));
    EXPECT_THAT(solver_->node->find_child(dup, 4)->position.y, Eq(n3->position.y));
    EXPECT_THAT(solver_->node->find_child(dup, 4)->position.z, Eq(n3->position.z));
    EXPECT_THAT(solver_->node->find_child(dup, 4)->rotation.x, Eq(n3->rotation.x));
    EXPECT_THAT(solver_->node->find_child(dup, 4)->rotation.y, Eq(n3->rotation.y));
    EXPECT_THAT(solver_->node->find_child(dup, 4)->rotation.z, Eq(n3->rotation.z));
    EXPECT_THAT(solver_->node->find_child(dup, 4)->rotation.w, Eq(n3->rotation.w));
    EXPECT_THAT(solver_->node->find_child(dup, 4)->user_data, IsNull());

    // Test some connectivity
    EXPECT_THAT(solver_->node->find_child(dup, 4)->parent, Eq(solver_->node->find_child(dup, 3)));
    EXPECT_THAT(dup->parent, IsNull());

    solver_->node->destroy(n1);
    solver_->node->destroy(dup);
}

TEST_F(NAME, duplicate_with_attachments_works)
{
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    ik_node_t* n3 = solver_->node->create_child(n2, 4);
    ik_node_t* n4 = solver_->node->create_child(n1, 5);

    ik_effector_t* eff = solver_->effector->create();
    ik_constraint_t* c = solver_->constraint->create();
    ik_pole_t* pole = solver_->pole->create();

    solver_->effector->attach(eff, n2);
    solver_->constraint->attach(c, n4);
    solver_->pole->attach(pole, n3);

    ik_node_t* dup = solver_->node->duplicate(n1, 1);

    ASSERT_THAT(solver_->node->find_child(dup, 2), NotNull());
    ASSERT_THAT(solver_->node->find_child(dup, 3), NotNull());
    ASSERT_THAT(solver_->node->find_child(dup, 4), NotNull());
    ASSERT_THAT(solver_->node->find_child(dup, 5), NotNull());

    EXPECT_THAT(solver_->node->find_child(dup, 3)->constraint, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 3)->pole, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 3)->effector, NotNull());
    EXPECT_THAT(solver_->node->find_child(dup, 3)->effector, Ne(eff));

    EXPECT_THAT(solver_->node->find_child(dup, 5)->effector, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 5)->pole, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 5)->constraint, NotNull());
    EXPECT_THAT(solver_->node->find_child(dup, 5)->constraint, Ne(c));

    EXPECT_THAT(solver_->node->find_child(dup, 4)->effector, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 4)->constraint, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 4)->pole, NotNull());
    EXPECT_THAT(solver_->node->find_child(dup, 4)->pole, Ne(pole));

    solver_->node->destroy(n1);
    solver_->node->destroy(dup);
}

TEST_F(NAME, duplicate_without_attachments_works)
{
    ik_node_t* n1 = solver_->node->create(2);
    ik_node_t* n2 = solver_->node->create_child(n1, 3);
    ik_node_t* n3 = solver_->node->create_child(n2, 4);
    ik_node_t* n4 = solver_->node->create_child(n1, 5);

    ik_effector_t* eff = solver_->effector->create();
    ik_constraint_t* c = solver_->constraint->create();
    ik_pole_t* pole = solver_->pole->create();

    solver_->effector->attach(eff, n2);
    solver_->constraint->attach(c, n4);
    solver_->pole->attach(pole, n3);

    ik_node_t* dup = solver_->node->duplicate(n1, 0);

    ASSERT_THAT(solver_->node->find_child(dup, 2), NotNull());
    ASSERT_THAT(solver_->node->find_child(dup, 3), NotNull());
    ASSERT_THAT(solver_->node->find_child(dup, 4), NotNull());
    ASSERT_THAT(solver_->node->find_child(dup, 5), NotNull());

    EXPECT_THAT(solver_->node->find_child(dup, 3)->constraint, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 3)->pole, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 3)->effector, IsNull());

    EXPECT_THAT(solver_->node->find_child(dup, 5)->effector, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 5)->pole, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 5)->constraint, IsNull());

    EXPECT_THAT(solver_->node->find_child(dup, 4)->effector, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 4)->constraint, IsNull());
    EXPECT_THAT(solver_->node->find_child(dup, 4)->pole, IsNull());

    solver_->node->destroy(n1);
    solver_->node->destroy(dup);
}

TEST_F(NAME, construct_NULL_death)
{
    EXPECT_DEATH(solver_->node->construct(NULL, 0), ".*");
}

TEST_F(NAME, destruct_and_destroy_NULL_death)
{
    EXPECT_DEATH(solver_->node->destruct(NULL), ".*");
    EXPECT_DEATH(solver_->node->destroy(NULL), ".*");
}

TEST_F(NAME, add_child_NULL_death)
{
    ik_node_t* n1 = solver_->node->create(1);
    ik_node_t* n2 = solver_->node->create(2);
    EXPECT_DEATH(solver_->node->add_child(n1, NULL), ".*");
    EXPECT_DEATH(solver_->node->add_child(NULL, n2), ".*");
    EXPECT_DEATH(solver_->node->add_child(NULL, NULL), ".*");
    solver_->node->destroy(n1);
    solver_->node->destroy(n2);
}

TEST_F(NAME, create_child_NULL_death)
{
    EXPECT_DEATH(solver_->node->create_child(NULL, 1), ".*");
}

TEST_F(NAME, unlink_NULL_death)
{
    EXPECT_DEATH(solver_->node->unlink(NULL), ".*");
}
