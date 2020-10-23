#include "ik/node.h"
#include "ik/chain_tree.h"
#include "ik/transform.h"
#include "ik/quat.inl"
#include "ik/vec3.inl"
#include <stddef.h>
#include <assert.h>
#include <string.h>

#if 0
/*
 * In all of the following algorithms, we iterate the nodes in a chain starting
 * with the node *after* the base node. This is because the base node is shared
 * by all chains in the list. If this current chain has children, then
 * transforming our tip is effectively transforming the base node of all of our
 * children. The only node left untransformed will be the root node, which
 * doesn't have to be transformed anyway, because its not relative to anything.
 */

/* ------------------------------------------------------------------------- */
static void
local_to_global_rotation_recursive(const struct ik_chain* chain)
{
    int idx = idx = ik_chain_length(chain) - 1;
    while (idx--)
    {
        struct ik_node_t* child  = ik_chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = ik_chain_get_node(chain, idx + 1);

        ik_quat_mul_quat(child->rotation.f, parent->rotation.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        local_to_global_rotation_recursive(child);
    CHAIN_END_EACH
}
static void
global_to_local_rotation_recursive(const struct ik_chain* chain)
{
    int idx;

    CHAIN_FOR_EACH_CHILD(chain, child)
        global_to_local_rotation_recursive(child);
    CHAIN_END_EACH

    for (idx = 0; idx != (int)ik_chain_length(chain) - 1; ++idx)
    {
        struct ik_node_t* child  = ik_chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = ik_chain_get_node(chain, idx + 1);

        ik_quat_nmul_quat(child->rotation.f, parent->rotation.f);
    }
}

/* ------------------------------------------------------------------------- */
static void
local_to_global_translation_recursive(const struct ik_chain* chain)
{
    int idx = ik_chain_length(chain) - 1;
    while (idx--)
    {
        struct ik_node_t* child  = ik_chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = ik_chain_get_node(chain, idx + 1);

        ik_vec3_nrotate(child->position.f, parent->rotation.f);
        ik_vec3_add_vec3(child->position.f, parent->position.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        local_to_global_translation_recursive(child);
    CHAIN_END_EACH
}
static void
global_to_local_translation_recursive(const struct ik_chain* chain)
{
    int idx;

    CHAIN_FOR_EACH_CHILD(chain, child)
        global_to_local_translation_recursive(child);
    CHAIN_END_EACH

    for (idx = 0; idx != (int)ik_chain_length(chain) - 1; idx++)
    {
        struct ik_node_t* child  = ik_chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = ik_chain_get_node(chain, idx + 1);

        ik_vec3_sub_vec3(child->position.f, parent->position.f);
        ik_vec3_rotate(child->position.f, parent->rotation.f);
    }
}

/* ------------------------------------------------------------------------- */
static void
local_to_global_recursive(const struct ik_chain* chain)
{
    int idx = ik_chain_length(chain) - 1;
    while (idx--)
    {
        struct ik_node_t* child  = ik_chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = ik_chain_get_node(chain, idx + 1);

        ik_vec3_nrotate(child->position.f, parent->rotation.f);
        ik_vec3_add_vec3(child->position.f, parent->position.f);
        ik_quat_mul_quat(child->rotation.f, parent->rotation.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        local_to_global_recursive(child);
    CHAIN_END_EACH
}
static void
global_to_local_recursive(const struct ik_chain* chain)
{
    int idx;

    CHAIN_FOR_EACH_CHILD(chain, child)
        global_to_local_recursive(child);
    CHAIN_END_EACH

    for (idx = 0; idx != (int)ik_chain_length(chain) - 1; ++idx)
    {
        struct ik_node_t* child  = ik_chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = ik_chain_get_node(chain, idx + 1);

        ik_quat_nmul_quat(child->rotation.f, parent->rotation.f);
        ik_vec3_sub_vec3(child->position.f, parent->position.f);
        ik_vec3_rotate(child->position.f, parent->rotation.f);
    }
}

/* ------------------------------------------------------------------------- */
static void
eff_local_to_global(struct ik_effector_t* eff, const struct ik_node_t* node)
{
    ik_vec3_nrotate(eff->actual_target.f, node->rotation.f);
    ik_vec3_add_vec3(eff->actual_target.f, node->position.f);
    ik_quat_mul_quat(eff->target_rotation.f, node->rotation.f);

    if (node->parent)
        eff_local_to_global(eff, node->parent);
}
static void
eff_global_to_local(struct ik_effector_t* eff, const struct ik_node_t* node)
{
    if (node->parent)
        eff_global_to_local(eff, node->parent);

    ik_quat_nmul_quat(eff->target_rotation.f, node->rotation.f);
    ik_vec3_sub_vec3(eff->actual_target.f, node->position.f);
    ik_vec3_rotate(eff->actual_target.f, node->rotation.f);
}
static void
eff_all_local_to_global(const struct ik_node_t* node_before_base, const struct ik_chain* chain)
{
    struct ik_node_t* tip = ik_chain_get_tip_node(chain);
    if (tip->effector)
        eff_local_to_global(tip->effector, node_before_base);

    CHAIN_FOR_EACH_CHILD(chain, child)
        eff_all_local_to_global(node_before_base, child);
    CHAIN_END_EACH
}
static void
eff_all_global_to_local(const struct ik_node_t* node_before_base, const struct ik_chain* chain)
{
    struct ik_node_t* tip = ik_chain_get_tip_node(chain);
    if (tip->effector)
        eff_global_to_local(tip->effector, node_before_base);

    CHAIN_FOR_EACH_CHILD(chain, child)
        eff_all_global_to_local(node_before_base, child);
    CHAIN_END_EACH
}

/* ------------------------------------------------------------------------- */
static void (*transform_table[8])(const struct ik_chain*) = {
    global_to_local_recursive,
    local_to_global_recursive,
    global_to_local_rotation_recursive,
    local_to_global_rotation_recursive,
    global_to_local_translation_recursive,
    local_to_global_translation_recursive,
    global_to_local_recursive,
    local_to_global_recursive,
};
static void (*eff_transform_table[2])(const struct ik_node_t*, const struct ik_chain* chain) = {
    eff_all_local_to_global,
    eff_all_global_to_local,
};

/* ------------------------------------------------------------------------- */
void
ik_transform_chain_list(const struct ik_algorithm_t* algorithm, uint8_t flags)
{
    VECTOR_FOR_EACH(&algorithm->chain_list, struct chain_t, chain)
        ik_transform_chain(chain, flags);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_transform_chain(struct ik_chain* chain, uint8_t flags)
{
    struct ik_node_t* base_node = ik_chain_get_base_node(chain);
    (*transform_table[flags])(chain);
    if (base_node->parent)
        (*eff_transform_table[flags & 0x01])(base_node->parent, chain);
}

/* ------------------------------------------------------------------------- */
void
ik_transform_pos_rot_l2g(ikreal pos[3], ikreal rot[4], const struct ik_node* tip, const struct ik_node* base)
{
    while (tip != base)
    {
        ik_vec3_rotate_quat_conj(pos, tip->rotation.f);
        ik_vec3_add_vec3(pos, tip->position.f);
        ik_quat_mul_quat(rot, tip->rotation.f);

        tip = tip->parent;
        assert(tip != NULL);
    }
}

/* ------------------------------------------------------------------------- */
void
ik_transform_pos_rot_g2l(ikreal pos[3], ikreal rot[4],  const struct ik_node* tip, const struct ik_node* base)
{
    assert(tip != NULL);
    if (tip != base)
        ik_transform_pos_rot_g2l(pos, rot, tip->parent, base);

    ik_quat_rmul_quat(tip->rotation.f, rot);
    ik_vec3_sub_vec3(pos, tip->position.f);
    ik_vec3_rotate_quat(pos, tip->rotation.f);
}

/* ------------------------------------------------------------------------- */
void
ik_transform_pos_l2g(ikreal pos[3], const struct ik_node* tip, const struct ik_node* base)
{
    while (tip != base)
    {
        ik_vec3_rotate_quat_conj(pos, tip->rotation.f);
        ik_vec3_add_vec3(pos, tip->position.f);

        tip = tip->parent;
        assert(tip != NULL);
    }
}

/* ------------------------------------------------------------------------- */
void
ik_transform_pos_g2l(ikreal pos[3], const struct ik_node* tip, const struct ik_node* base)
{
    assert(tip != NULL);
    if (tip == base)
        return;
    ik_transform_pos_g2l(pos, tip->parent, base);

    ik_vec3_sub_vec3(pos, tip->position.f);
    ik_vec3_rotate_quat(pos, tip->rotation.f);
}

/* ------------------------------------------------------------------------- */
void
ik_transform_node_section_l2g(struct ik_node* tip, const struct ik_node* base)
{
    struct ik_node* parent;

    assert(tip != NULL);
    if (tip->parent == base)
        return;
    ik_transform_node_section_l2g(tip->parent, base);

    parent = tip->parent;
    ik_vec3_rotate_quat_conj(tip->position.f, parent->rotation.f);
    ik_vec3_add_vec3(tip->position.f, parent->position.f);
    ik_quat_mul_quat(tip->rotation.f, parent->rotation.f);
}

/* ------------------------------------------------------------------------- */
void
ik_transform_node_section_g2l(struct ik_node* tip, const struct ik_node* base)
{
    while (tip->parent != base)
    {
        struct ik_node* parent = tip->parent;
        assert(parent != NULL);

        ik_vec3_sub_vec3(tip->position.f, parent->position.f);
        ik_vec3_rotate_quat(tip->position.f, parent->rotation.f);
        ik_quat_rmul_quat(tip->rotation.f, parent->rotation.f);

        tip = tip->parent;
        assert(tip != NULL);
    }
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Nodes that have siblings will have a translation that is not [0, 0, 1],
 * and their parent node will have a rotation that is an average of all
 * sibling rotations.
 *
 * Need to calculate the parent node rotation that would place the child
 * node at [0, 0, 1] and store that rotation in the child node.
 */
#if 0
static void
unaverage_sibling_segments(const struct ik_chain* chain,
                           union ik_quat** rotations_store)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        unaverage_sibling_segments(child, rotations_store);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        struct ik_node* tail = chain_get_node(child_chain, chain_node_count(child_chain) - 1);
        struct ik_node* head = chain_get_node(child_chain, chain_node_count(child_chain) - 2);

        ik_quat_angle_of((**rotations_store).f, head->position.f);             /* delta to correct rotation */
        ik_vec3_set(head->position.f, 0, 0, ik_vec3_length(head->position.f)); /* rotate to [0,0,1] -- this is faster than ik_quat_mul_quat() */
        ik_quat_mul_quat_conj(head->rotation.f, (**rotations_store).f);        /* rotate tip in opposite direction so all children retain their orientation */
        ik_quat_mul_quat((**rotations_store).f, tail->rotation.f);             /* Add average rotation to delta, converting the segment's rotation into an absolute one */

        (*rotations_store)++;
    CHAIN_END_EACH

    CHAIN_FOR_EACH_DEAD_NODE(chain, head)
        struct ik_node* tail = chain_get_tip_node(chain);                      /* Our tip node is the dead node's tail */

        ik_quat_angle_of((**rotations_store).f, head->position.f);             /* delta to correct rotation */
        ik_vec3_set(head->position.f, 0, 0, ik_vec3_length(head->position.f)); /* rotate to [0,0,1] -- this is faster than ik_quat_mul_quat() */
        ik_quat_mul_quat_conj(head->rotation.f, (**rotations_store).f);        /* rotate tip in opposite direction so all children retain their orientation */
        ik_quat_mul_quat((**rotations_store).f, tail->rotation.f);             /* Add average rotation to delta, converting the segment's rotation into an absolute one */

        (*rotations_store)++;
    CHAIN_END_EACH
}
static void
copy_rotations_to_children(const struct ik_chain* chain,
                           union ik_quat** rotations_store)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        copy_rotations_to_children(child, rotations_store);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        union ik_quat tmp;
        struct ik_node* head = chain_get_node(child_chain, chain_node_count(child_chain) - 2);
        struct ik_node* tip  = chain_get_tip_node(child_chain);

        tmp = **rotations_store;
        **rotations_store = tip->rotation;
        CHAIN_FOR_EACH_SEGMENT_RANGE(child_chain, parent, child, 0, chain_segment_count(child_chain) - 1)
            child->rotation = parent->rotation;
        CHAIN_END_EACH
        head->rotation = tmp;

        (*rotations_store)++;
    CHAIN_END_EACH

    CHAIN_FOR_EACH_DEAD_NODE(chain, head)
        (void)head;
        (*rotations_store)++;
    CHAIN_END_EACH
}
void
ik_transform_chain_to_segmental_representation(struct ik_chain* root,
                                               union ik_quat* intermediate_rotations,
                                               int num_intermediate_rotations)
{
    union ik_quat* rotations_store;

    /*
     * Copy all effector node rotations into a pre-allocated array stored in the
     * solver object so we can restore them later. They will be overwritten
     * by the parent node rotation in the next section of code.
     */
    rotations_store = intermediate_rotations;
    unaverage_sibling_segments(root, &rotations_store);
    rotations_store = intermediate_rotations;
    copy_rotations_to_children(root, &rotations_store);

    *rotations_store = chain_get_tip_node(root)->rotation;
    CHAIN_FOR_EACH_SEGMENT(root, parent, child)
        child->rotation = parent->rotation;
    CHAIN_END_EACH
}

/* ------------------------------------------------------------------------- */
static void
copy_rotations_to_parent(const struct ik_chain* chain,
                         union ik_quat** rotations_store)
{
    CHAIN_FOR_EACH_DEAD_NODE_R(chain, head)
        (void)head;
        (*rotations_store)--;
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD_R(chain, child_chain)
        union ik_quat tmp;
        struct ik_node* head = chain_get_node(child_chain, chain_node_count(child_chain) - 2);
        struct ik_node* tip  = chain_get_tip_node(child_chain);

        (*rotations_store)--;

        tmp = **rotations_store;
        **rotations_store = head->rotation;
        CHAIN_FOR_EACH_SEGMENT_RANGE_R(child_chain, parent, child, 0, chain_segment_count(child_chain) - 1)
            parent->rotation = child->rotation;
        CHAIN_END_EACH
        tip->rotation = tmp;
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD_R(chain, child)
        copy_rotations_to_parent(child, rotations_store);
    CHAIN_END_EACH
}
static void
average_sibling_segments(const struct ik_chain* chain,
                         union ik_quat** rotations_store)
{
    union ik_quat* rotations;
    ikreal* avg = chain_get_tip_node(chain)->rotation.f;

    if (chain_child_count(chain) == 0 && chain_dead_node_count(chain) == 0)
        return;

    ik_quat_set(avg, 0, 0, 0, 0);
    rotations = *rotations_store;
    CHAIN_FOR_EACH_DEAD_NODE_R(chain, head)
        (void)head;
        rotations--;
        ik_quat_ensure_positive_sign(rotations->f);
        ik_quat_add_quat(avg, rotations->f);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD_R(chain, child_chain)
        rotations--;
        ik_quat_ensure_positive_sign(rotations->f);
        ik_quat_add_quat(avg, rotations->f);
    CHAIN_END_EACH

    /* Average */
    ik_quat_div_scalar(avg, chain_child_count(chain) + chain_dead_node_count(chain));
    ik_quat_normalize(avg);

    /* Calculate new translations */
    CHAIN_FOR_EACH_DEAD_NODE_R(chain, head)
        (*rotations_store)--;
        ik_quat_conj_rmul_quat(avg, (**rotations_store).f);
        ik_vec3_rotate_quat(head->position.f, (**rotations_store).f);
        ik_quat_mul_quat(head->rotation.f, (**rotations_store).f);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD_R(chain, child_chain)
        struct ik_node* head = chain_get_node(child_chain, chain_node_count(child_chain) - 2);

        (*rotations_store)--;
        ik_quat_conj_rmul_quat(avg, (**rotations_store).f);
        ik_vec3_rotate_quat(head->position.f, (**rotations_store).f);
        ik_quat_mul_quat(head->rotation.f, (**rotations_store).f);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD_R(chain, child)
        average_sibling_segments(child, rotations_store);
    CHAIN_END_EACH
}
void
ik_transform_chain_to_nodal_representation(struct ik_chain* root,
                                           union ik_quat* intermediate_rotations,
                                           int num_intermediate_rotations)
{
    union ik_quat* rotations_store;

    CHAIN_FOR_EACH_SEGMENT_R(root, parent, child)
        parent->rotation = child->rotation;
    CHAIN_END_EACH
    rotations_store = intermediate_rotations + num_intermediate_rotations - 1;
    chain_get_tip_node(root)->rotation = *rotations_store;

    copy_rotations_to_parent(root, &rotations_store);
    rotations_store = intermediate_rotations + num_intermediate_rotations - 1;
    average_sibling_segments(root, &rotations_store);
}
#endif
