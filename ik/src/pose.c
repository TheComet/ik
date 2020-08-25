#include "ik/pose.h"
#include "ik/node.h"
#include <stddef.h>

struct ik_node_state
{
    union ik_quat rotation;
    union ik_vec3 position;
};

#define IK_POSE_OFFSET \
    IK_ALIGN_TO_CPU_WORD_SIZE(sizeof(struct ik_pose))

/* ------------------------------------------------------------------------- */
struct ik_pose*
ik_pose_alloc(const struct ik_node* root)
{
    uint32_t node_count = ik_node_count(root);
    struct ik_pose* state = (struct ik_pose*)ik_refcounted_alloc(
        IK_POSE_OFFSET + sizeof(struct ik_node_state) * node_count, NULL);
    if (state == NULL)
        return NULL;

#ifdef DEBUG
    state->node_count = node_count;
#endif

    return state;
}

/* ------------------------------------------------------------------------- */
static void
save_pose(const struct ik_node* node, struct ik_node_state** data)
{
    NODE_FOR_EACH(node, child)
        save_pose(child, data);
    NODE_END_EACH

    (*data)->position = node->position;
    (*data)->rotation = node->rotation;
    (*data)++;
}
void
ik_pose_save(struct ik_pose* state, const struct ik_node* root)
{
    struct ik_node_state* data = (struct ik_node_state*)((uintptr_t)state + IK_POSE_OFFSET);
#ifdef DEBUG
    assert(state->node_count == ik_node_count(root));
#endif
    save_pose(root, &data);
}

/* ------------------------------------------------------------------------- */
static void
restore_pose(struct ik_node* node, struct ik_node_state** data)
{
    NODE_FOR_EACH(node, child)
        restore_pose(child, data);
    NODE_END_EACH

    node->position = (*data)->position;
    node->rotation = (*data)->rotation;
    (*data)++;
}
void
ik_pose_apply(const struct ik_pose* state, struct ik_node* root)
{
    struct ik_node_state* data = (struct ik_node_state*)((uintptr_t)state + IK_POSE_OFFSET);
#ifdef DEBUG
    assert(state->node_count == ik_node_count(root));
#endif
    restore_pose(root, &data);
}
