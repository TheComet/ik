#include "gmock/gmock.h"

#include "ik/effector.h"
#include "ik/node.h"
#include "ik/solver.h"

#define NAME solver_fabrik

using namespace ::testing;

const double pi = std::atan(1.0) * 4;

class NAME : public Test
{
public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

protected:
};

static void buildTreeLongChainsRecurse(ik_solver* solver, ik_node* parent, int depth, int* guid)
{
    ik_node* child1 = ik_node_create(ik_guid((*guid)++));
    ik_node* child2 = ik_node_create(ik_guid((*guid)++));
    ik_node* child3 = ik_node_create(ik_guid((*guid)++));
    ik_node* child4 = ik_node_create(ik_guid((*guid)++));
    ik_node* child5 = ik_node_create(ik_guid((*guid)++));
    ik_node* child6 = ik_node_create(ik_guid((*guid)++));

    ik_node_link(parent, child1);
    ik_node_link(child1, child2);
    ik_node_link(child2, child3);

    ik_node_link(parent, child4);
    ik_node_link(child4, child5);
    ik_node_link(child5, child6);

    if(depth > 0)
    {
        buildTreeLongChainsRecurse(solver, child3, depth-1, guid);
        buildTreeLongChainsRecurse(solver, child6, depth-1, guid);
    }
    else
    {
        ik_node_create_effector(child3);
        ik_node_create_effector(child6);
    }
}
static void buildTreeLongChains(ik_solver* solver, int depth)
{
    int guid = 0;
    ik_node* root = ik_node_create(ik_guid(guid++));
    buildTreeLongChainsRecurse(solver, root, depth, &guid);
}

static void buildTreeShortChainsRecurse(ik_solver* solver, ik_node* parent, int depth, int* guid)
{
    ik_node* child1 = ik_node_create(ik_guid((*guid)++));
    ik_node* child2 = ik_node_create(ik_guid((*guid)++));
    ik_node_link(parent, child1);
    ik_node_link(parent, child2);

    if(depth > 0)
    {
        buildTreeShortChainsRecurse(solver, child1, depth-1, guid);
        buildTreeShortChainsRecurse(solver, child2, depth-1, guid);
    }
    else
    {
        ik_node_create_effector(child1);
        ik_node_create_effector(child2);
    }
}
static void buildTreeShortChains(ik_solver* solver, int depth)
{
    int guid = 0;
    ik_node* root = ik_node_create(ik_guid(guid++));
    buildTreeShortChainsRecurse(solver, root, depth, &guid);
}

TEST_F(NAME, todo)
{

}
