#include "cstructures/memory.h"
#include "ik/algorithm.h"
#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/solver_fabrik.h"
#include <stddef.h>
#include <string.h>

/* ------------------------------------------------------------------------- *
static void
initial_transform_local_to_global(struct ik_solver_fabrik_t* solver)
{
    const struct ik_node_data_view_t* ndv = &solver->ndv;
    const struct ik_node_data_t* nda = ndv->node_data;

    IK_NDV_FOR(ndv, idx)
        union ik_transform_t* cur_trans = &solver->initial_transforms[idx];
        union ik_transform_t* parent_trans = &solver->initial_transforms[nda->pre_order.parent_idx[idx]];

        ik_vec3_nrotate(cur_trans->t.position.f, parent_trans->t.rotation.f);
        ik_vec3_add_vec3(cur_trans->t.position.f, parent_trans->t.position.f);
        ik_quat_mul_quat(cur_trans->t.rotation.f, parent_trans->t.rotation.f);
    IK_NDV_END
}

* ------------------------------------------------------------------------- *
static void
store_initial_transform(struct ik_solver_fabrik_t* solver)
{
    const struct ik_node_data_view_t* ndv = &solver->ndv;
    const struct ik_node_data_t* nda = ndv->node_data;

    union ik_transform_t* buf_ptr = solver->initial_transforms;
    IK_NDV_FOR(ndv, idx)
        *buf_ptr++ = nda->transform[idx];
    IK_NDV_END

    initial_transform_local_to_global(solver);
}*/

/* ------------------------------------------------------------------------- */
static void zero_transform_stack(struct ik_solver_fabrik_t* solver)
{
    int idx = (int)solver->transform_stack_depth;
    while (idx--)
        ik_vec3_set_zero(solver->transform_stack[idx].f);
}

/* ------------------------------------------------------------------------- */
static void
solve_forwards(struct ik_solver_fabrik_t* solver)
{
    union ik_vec3_t target_pos;
    union ik_vec3_t target_dir;
    union ik_quat_t parent_rot;
    const struct ik_node_data_view_t* ndv = &solver->ndv;
    const struct ik_node_data_t* nda = ndv->node_data;
    const uint16_t features = solver->algorithm->features;

    zero_transform_stack(solver);

    IK_NDV_FOR_R(ndv, idx)
        ikreal_t* this_pos = nda->transform[idx].t.position.f;
        ikreal_t* this_rot = nda->transform[idx].t.rotation.f;

        if (nda->commands[idx] & CMD_LOAD_EFFECTOR)
        {
            ik_vec3_copy(target_pos.f, nda->effector[idx]->actual_target.f);

            if (features & IK_SOLVER_TARGET_ROTATIONS)
            {
                ik_vec3_set(target_dir.f, 0, 0, 1);
                ik_vec3_rotate_quat(target_dir.f, nda->effector[idx]->target_rotation.f);
            }
        }
        else if (nda->child_count[idx] > 1)
        {
        }
        else
        {
        }

        /* Transform target into parent space, so it is in the same space as the
         * current node */
        ik_vec3_nrotate_quat(target_pos.f, this_rot);
        ik_vec3_add_vec3(target_pos.f, this_pos);
        ik_vec3_rotate_quat(target_dir.f, this_rot);

        /* Lerp between target vector and effector direction depending on this
         * node's rotation weight to get the "real" target direction */
        if (features & IK_SOLVER_TARGET_ROTATIONS)
        {
            ik_vec3_normalize(target_pos.f);
            ik_vec3_sub_vec3(target_pos.f, target_dir.f);
            ik_vec3_mul_scalar(target_pos.f, nda->rotation_weight[idx]);
            ik_vec3_add_vec3(target_pos.f, target_dir.f);
        }

        /* Calculate rotation parent node would need to have so it would point
         * towards the target position */
        ik_quat_angle(parent_rot.f, this_pos, target_pos.f);
        ik_quat_nmul_quat(parent_rot.f, nda->transform[nda->parent_idx[idx]].t.rotation.f);

        /* Apply constraint to this new rotation, if any. The constraint applies
         * to the rotation of the parent node, not to the current node */
        if ((features & IK_SOLVER_CONSTRAINTS) && nda->constraint[idx] != NULL)
        {
            union ik_quat_t delta_rot;
            nda->constraint[idx]->apply(nda->constraint[idx], delta_rot.f, parent_rot.f);
        }

        /* Calculate new target for the parent node */

    IK_NDV_END
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_fabrik_init(struct ik_solver_fabrik_t* solver)
{
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_fabrik_deinit(struct ik_solver_fabrik_t* solver)
{
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_fabrik_prepare(struct ik_solver_fabrik_t* solver)
{
    uint32_t max_children;
    const struct ik_node_data_view_t* ndv = &solver->ndv;
    const struct ik_node_data_t* nda = ndv->node_data;

    /*
     * The solver needs a small stack to push/pop transformations as it
     * iterates the tree.
     * TODO: Add support for alloca(). If the stack is small enough and the
     * platform supports alloca(), leave this as NULL.
     */

    XFREE(solver->transform_stack);
    solver->transform_stack = NULL;

    /* Determine the maximum number of child chains present in the tree */
    max_children = 0;
    IK_NDV_FOR(ndv, idx)
        uint32_t child_count = nda->child_count[idx];
        if (max_children < child_count)
            max_children = child_count;
    IK_NDV_END

    /* Simple trees don't have more than 1 child and don't need a stack buffer */
    if (max_children > 1)
    {
        solver->transform_stack = MALLOC(sizeof(union ik_transform_t) * max_children);
        if (solver->transform_stack == NULL)
        {
            ik_log_fatal("Failed to allocate solver stack: Ran out of memory");
            return IK_ERR_OUT_OF_MEMORY;
        }
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_fabrik_solve(struct ik_solver_fabrik_t* solver)
{
    const struct ik_algorithm_t* algo = solver->algorithm;
    int iteration = algo->max_iterations;
    ikreal_t tolerance_squared = algo->tolerance * algo->tolerance;

    /*
     * Joint rotations are calculated by comparing positional differences
     * before and after solving the tree. This comparison needs to occur in
     * global space (doesn't work in local as far as I can see). Store the
     * positions and locations before solving for later.
     *
    if (algo->features & IK_SOLVER_JOINT_ROTATIONS)
        store_initial_transform(solver);*/

    while (iteration--)
    {
        solve_forwards(solver);
        (void)tolerance_squared;
    }

    /*if (solver->features & IK_ALGORITHM_JOINT_ROTATIONS)
        calculate_joint_rotations(solver);*/

    return IK_OK;
}
