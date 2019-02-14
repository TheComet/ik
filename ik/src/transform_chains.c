#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/node.h"
#include "ik/quat.h"
#include "ik/solverdef.h"
#include "ik/transform.h"
#include "ik/vec3.h"
#include <stddef.h>
#include <assert.h>
#include <string.h>

/* Need the solver structure for ik_transform_chain_list */
struct ik_solver_t
{
    SOLVER_HEAD
};

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
local_to_global_rotation_recursive(const struct chain_t* chain)
{
    int idx = idx = chain_length(chain) - 1;
    while (idx--)
    {
        struct ik_node_t* child  = chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = chain_get_node(chain, idx + 1);

        ik_quat_mul_quat(child->rotation.f, parent->rotation.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        local_to_global_rotation_recursive(child);
    CHAIN_END_EACH
}
static void
global_to_local_rotation_recursive(const struct chain_t* chain)
{
    int idx;

    CHAIN_FOR_EACH_CHILD(chain, child)
        global_to_local_rotation_recursive(child);
    CHAIN_END_EACH

    for (idx = 0; idx != (int)chain_length(chain) - 1; ++idx)
    {
        struct ik_node_t* child  = chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = chain_get_node(chain, idx + 1);

        ik_quat_nmul_quat(child->rotation.f, parent->rotation.f);
    }
}

/* ------------------------------------------------------------------------- */
static void
local_to_global_translation_recursive(const struct chain_t* chain)
{
    int idx = chain_length(chain) - 1;
    while (idx--)
    {
        struct ik_node_t* child  = chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = chain_get_node(chain, idx + 1);

        ik_vec3_nrotate(child->position.f, parent->rotation.f);
        ik_vec3_add_vec3(child->position.f, parent->position.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        local_to_global_translation_recursive(child);
    CHAIN_END_EACH
}
static void
global_to_local_translation_recursive(const struct chain_t* chain)
{
    int idx;

    CHAIN_FOR_EACH_CHILD(chain, child)
        global_to_local_translation_recursive(child);
    CHAIN_END_EACH

    for (idx = 0; idx != (int)chain_length(chain) - 1; idx++)
    {
        struct ik_node_t* child  = chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = chain_get_node(chain, idx + 1);

        ik_vec3_sub_vec3(child->position.f, parent->position.f);
        ik_vec3_rotate(child->position.f, parent->rotation.f);
    }
}

/* ------------------------------------------------------------------------- */
static void
local_to_global_recursive(const struct chain_t* chain)
{
    int idx = chain_length(chain) - 1;
    while (idx--)
    {
        struct ik_node_t* child  = chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = chain_get_node(chain, idx + 1);

        ik_vec3_nrotate(child->position.f, parent->rotation.f);
        ik_vec3_add_vec3(child->position.f, parent->position.f);
        ik_quat_mul_quat(child->rotation.f, parent->rotation.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        local_to_global_recursive(child);
    CHAIN_END_EACH
}
static void
global_to_local_recursive(const struct chain_t* chain)
{
    int idx;

    CHAIN_FOR_EACH_CHILD(chain, child)
        global_to_local_recursive(child);
    CHAIN_END_EACH

    for (idx = 0; idx != (int)chain_length(chain) - 1; ++idx)
    {
        struct ik_node_t* child  = chain_get_node(chain, idx + 0);
        struct ik_node_t* parent = chain_get_node(chain, idx + 1);

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
eff_all_local_to_global(const struct ik_node_t* node_before_base, const struct chain_t* chain)
{
    struct ik_node_t* tip = chain_get_tip_node(chain);
    if (tip->effector)
        eff_local_to_global(tip->effector, node_before_base);

    CHAIN_FOR_EACH_CHILD(chain, child)
        eff_all_local_to_global(node_before_base, child);
    CHAIN_END_EACH
}
static void
eff_all_global_to_local(const struct ik_node_t* node_before_base, const struct chain_t* chain)
{
    struct ik_node_t* tip = chain_get_tip_node(chain);
    if (tip->effector)
        eff_global_to_local(tip->effector, node_before_base);

    CHAIN_FOR_EACH_CHILD(chain, child)
        eff_all_global_to_local(node_before_base, child);
    CHAIN_END_EACH
}

/* ------------------------------------------------------------------------- */
static void (*transform_table[8])(const struct chain_t*) = {
    global_to_local_recursive,
    local_to_global_recursive,
    global_to_local_rotation_recursive,
    local_to_global_rotation_recursive,
    global_to_local_translation_recursive,
    local_to_global_translation_recursive,
    global_to_local_recursive,
    local_to_global_recursive,
};
static void (*eff_transform_table[2])(const struct ik_node_t*, const struct chain_t* chain) = {
    eff_all_local_to_global,
    eff_all_global_to_local,
};

/* ------------------------------------------------------------------------- */
void
ik_transform_chain_list(const struct ik_solver_t* solver, uint8_t flags)
{
    VECTOR_FOR_EACH(&solver->chain_list, struct chain_t, chain)
        ik_transform_chain(chain, flags);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_transform_chain(struct chain_t* chain, uint8_t flags)
{
    struct ik_node_t* base_node = chain_get_base_node(chain);
    (*transform_table[flags])(chain);
    if (base_node->parent)
        (*eff_transform_table[flags & 0x01])(base_node->parent, chain);
}
