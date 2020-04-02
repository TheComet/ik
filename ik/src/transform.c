#include "ik/node.h"
#include "ik/transform.h"
#include "ik/vec3.h"
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
#endif

/* ------------------------------------------------------------------------- */
void
ik_transform_pos_rot_l2g(ikreal pos[3], ikreal rot[4], const struct ik_node* tip, const struct ik_node* base)
{
    while (tip != base)
    {
        ik_vec3_nrotate_quat(pos, tip->rotation.f);
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

    ik_quat_nmul_quat(rot, tip->rotation.f);
    ik_vec3_sub_vec3(pos, tip->position.f);
    ik_vec3_rotate_quat(pos, tip->rotation.f);
}

/* ------------------------------------------------------------------------- */
void
ik_transform_pos_l2g(ikreal pos[3], const struct ik_node* tip, const struct ik_node* base)
{
    while (tip != base)
    {
        ik_vec3_nrotate_quat(pos, tip->rotation.f);
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
    if (tip == base)
        return;
    ik_transform_node_section_l2g(tip->parent, base);

    parent = tip->parent;
    ik_vec3_nrotate_quat(tip->position.f, parent->rotation.f);
    ik_vec3_add_vec3(tip->position.f, parent->position.f);
    ik_quat_mul_quat(tip->rotation.f, parent->rotation.f);
}

/* ------------------------------------------------------------------------- */
void
ik_transform_node_section_g2l(struct ik_node* tip, const struct ik_node* base)
{
    while (tip != base)
    {
        struct ik_node* parent = tip->parent;
        assert(parent != NULL);

        ik_vec3_sub_vec3(tip->position.f, parent->position.f);
        ik_vec3_rotate_quat(tip->position.f, parent->rotation.f);
        ik_quat_nmul_quat(tip->rotation.f, parent->rotation.f);

        tip = tip->parent;
    }
}
