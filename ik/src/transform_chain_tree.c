#include "ik/bst_vector.h"
#include "ik/chain.h"
#include "ik/node.h"
#include "ik/quat.h"
#include "ik/transform.h"
#include "ik/vec3.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
static void
local_to_global_recursive(chain_t* chain,
                          uintptr_t off_position, uintptr_t off_rotation,
                          vec3_t acc_pos, quat_t acc_rot)
{
    vec3_t position;
    quat_t rotation;

    /* Iterate nodes in chain, starting at the base (and excluding the base) */
    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        ik_node_t* node = chain_get_node(chain, idx);

        /* Get position and rotation fields in ik_node_t struct */
        vec3_t* node_position = (vec3_t*)((char*)node + off_position);
        quat_t* node_rotation = (quat_t*)((char*)node + off_rotation);

        quat_rotate_vec(node_position->f, acc_rot.f);
        position = *node_position;
        vec3_add_vec3(node_position->f, acc_pos.f);
        vec3_add_vec3(acc_pos.f, position.f);

        rotation = *node_rotation;
        quat_mul_quat(node_rotation->f, acc_rot.f);
        quat_mul_quat(acc_rot.f, rotation.f);
    }

    /* Recurse into child chains */
    CHAIN_FOR_EACH_CHILD(chain, child)
        local_to_global_recursive(child, off_position, off_rotation, acc_pos, acc_rot);
    CHAIN_END_EACH
}
static void
global_to_local_recursive(chain_t* chain,
                          uintptr_t off_position, uintptr_t off_rotation,
                          vec3_t acc_pos, quat_t acc_rot)
{
    /* Iterate nodes in chain, starting at the base (and excluding the base) */
    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        ik_node_t* node = chain_get_node(chain, idx);

        quat_t inv_rotation = acc_rot;
        /* Get position and rotation fields in ik_node_t struct */
        vec3_t* node_position = (vec3_t*)((char*)node + off_position);
        quat_t* node_rotation = (quat_t*)((char*)node + off_rotation);

        quat_conj(inv_rotation.f);
        quat_mul_quat(node_rotation->f, inv_rotation.f);
        quat_mul_quat(acc_rot.f, node_rotation->f);

        vec3_sub_vec3(node_position->f, acc_pos.f);
        vec3_add_vec3(acc_pos.f, node_position->f);
        quat_rotate_vec(node_position->f, inv_rotation.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        global_to_local_recursive(child, off_position, off_rotation, acc_pos, acc_rot);
    CHAIN_END_EACH
}
static void
local_to_global_rotation_recursive(chain_t* chain, uintptr_t off_rotation, quat_t acc_rot)
{
    /* Iterate nodes in chain, starting at the base (and excluding the base) */
    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        ik_node_t* node = chain_get_node(chain, idx);

        /* Get rotation field in ik_node_t struct */
        quat_t* node_rotation = (quat_t*)((char*)node + off_rotation);

        quat_t rotation = *node_rotation;
        quat_mul_quat(node_rotation->f, acc_rot.f);
        quat_mul_quat(acc_rot.f, rotation.f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        local_to_global_rotation_recursive(child, off_rotation, acc_rot);
    CHAIN_END_EACH
}
static void
global_to_local_rotation_recursive(chain_t* chain, uintptr_t off_rotation, quat_t acc_rot)
{
    /* Iterate nodes in chain, starting at the base (and excluding the base) */
    int idx = chain_length(chain) - 1;
    assert(idx > 0);
    while (idx--)
    {
        ik_node_t* node = chain_get_node(chain, idx);

        /* Get rotation field in ik_node_t struct */
        quat_t* node_rotation = (quat_t*)((char*)node + off_rotation);

        quat_t inv_rotation = acc_rot;
        quat_conj(inv_rotation.f);
        quat_mul_quat(node_rotation->f, inv_rotation.f);
        quat_mul_quat(acc_rot.f, node_rotation->f);
    }

    CHAIN_FOR_EACH_CHILD(chain, child)
        global_to_local_rotation_recursive(child, off_rotation, acc_rot);
    CHAIN_END_EACH
}

typedef void (*transform_both_func)(chain_t*,uintptr_t,uintptr_t,vec3_t,quat_t);
typedef void (*transform_rot_func) (chain_t*,uintptr_t,quat_t);

/* ------------------------------------------------------------------------- */
static void
do_transform(chain_t* chain, uint8_t flags,
             transform_both_func transform_both,
             transform_rot_func transform_rot)
{
    ik_node_t* base_node;

    assert(chain_length(chain) >= 2);
    base_node = chain_get_base_node(chain);

    if (flags & TRANSFORM_ROTATIONS_ONLY)
    {
        if (flags & TRANSFORM_ACTIVE)
            transform_rot(chain,
                          offsetof(ik_node_t, rotation),
                          base_node->rotation);
        if (flags & TRANSFORM_ORIGINAL)
            transform_rot(chain,
                          offsetof(ik_node_t, original_rotation),
                          base_node->original_rotation);
    }
    else
    {
        if (flags & TRANSFORM_ACTIVE)
            transform_both(chain,
                           offsetof(ik_node_t, position), offsetof(ik_node_t, rotation),
                           base_node->position, base_node->rotation);
        if (flags & TRANSFORM_ORIGINAL)
            transform_both(chain,
                           offsetof(ik_node_t, original_position), offsetof(ik_node_t, original_rotation),
                           base_node->original_position, base_node->original_rotation);
    }
}

/* ------------------------------------------------------------------------- */
void
ik_chain_local_to_global(chain_t* chain, uint8_t flags)
{
    do_transform(chain, flags, local_to_global_recursive, local_to_global_rotation_recursive);
}

/* ------------------------------------------------------------------------- */
void
ik_chain_global_to_local(chain_t* chain, uint8_t flags)
{
    do_transform(chain, flags, global_to_local_recursive, global_to_local_rotation_recursive);
}

/* ------------------------------------------------------------------------- */
void
ik_chains_local_to_global(const vector_t* chains, uint8_t flags)
{
    VECTOR_FOR_EACH(chains, chain_t, chain)
        ik_chain_local_to_global(chain, flags);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_chains_global_to_local(const vector_t* chains, uint8_t flags)
{
    VECTOR_FOR_EACH(chains, chain_t, chain)
        ik_chain_global_to_local(chain, flags);
    VECTOR_END_EACH
}
