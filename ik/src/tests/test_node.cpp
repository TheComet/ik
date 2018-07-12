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
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrEq(""));
#else
    EXPECT_THAT(internal::GetCapturedStdout().c_str(), StrEq(""));
#endif
    solver_->node->destroy(n1);
}