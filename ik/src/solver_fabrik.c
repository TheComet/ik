#include "ik/chain_tree.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/bone.h"
#include "ik/solver.h"
#include "ik/subtree.h"
#include "ik/transform.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"

#include "cstructures/memory.h"

#include <stddef.h>
#include <math.h>

struct ik_solver_fabrik
{
    IK_SOLVER_HEAD

    struct ik_chain chain_tree;
    struct ik_chain** effector_chains;
    union ik_vec3* target_positions;
    union ik_quat* deltas;
    ikreal* lengths;

    int num_effectors;
};

/* ------------------------------------------------------------------------- */
static int
validate_poles_recursive(const struct ik_solver_fabrik* solver, const struct ik_chain* chain)
{
    int poles_found = 0;
    CHAIN_FOR_EACH_CHILD(chain, child)
        poles_found += validate_poles_recursive(solver, child);
    CHAIN_END_EACH

    /* Pole target constraints should only be attached to the tip bone of each
     * chain. */
    CHAIN_FOR_EACH_BONE(chain, bone)
        if (bone == chain_get_tip_bone(chain))
            continue;
        if (bone->pole != NULL)
        {
            ik_log_printf(IK_WARN, "FABRIK: Pole attached to bone (address: 0x%p) has no effect and will be ignored.", bone);
            poles_found++;
        }
    CHAIN_END_EACH

    return poles_found;
}
static void
validate_poles(const struct ik_solver_fabrik* solver)
{
    if (validate_poles_recursive(solver, &solver->chain_tree))
    {
        ik_log_printf(IK_WARN, "FABRIK: Poles only make sense when attached to the end of chains, such as effector bones, or bones with multiple children.");
    }
}

/* ------------------------------------------------------------------------- */
static void
store_effector_bones_recursive(struct ik_chain*** effector_chains_store, struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child)
        store_effector_bones_recursive(effector_chains_store, child);
    CHAIN_END_EACH

    if (chain_child_count(chain) == 0)
    {
        **effector_chains_store = chain;
        (*effector_chains_store)++;
    }
}
static void
store_effector_chains(struct ik_solver_fabrik* solver)
{
    struct ik_chain** effector_chains_store = solver->effector_chains;
    store_effector_bones_recursive(&effector_chains_store, &solver->chain_tree);

    /* sanity check */
    assert(effector_chains_store - solver->effector_chains == solver->num_effectors);
}

/* ------------------------------------------------------------------------- */
static void
calculate_target_data(struct ik_solver_fabrik* solver)
{
    int i;

    for (i = 0; i != solver->num_effectors; ++i)
    {
        union ik_vec3 tip_pos;
        const struct ik_bone* base_bone = chain_get_base_bone(&solver->chain_tree);
        const struct ik_bone* root_bone = solver->root_bone;
        const struct ik_chain* eff_chain = solver->effector_chains[i];
        const struct ik_bone* tip_bone = chain_get_tip_bone(eff_chain);
        struct ik_effector* eff = tip_bone->effector;
        ikreal* target = solver->target_positions[i].f;

        /*
         * The "actual" target position is calculated once and must be stored in
         * a space outside of the bones being solved. It is retrieved by the
         * FABRIK algorithm each iteration.
         *
         * The actual target position depends on the effector target position
         * and the effector weight. Most of the time it is just equal to the
         * effector's target position.
         *
         * Note that sometimes the parent of root_bone is not equal to the
         * parent of base_bone. We transform the target position up to the
         * parent of base_bone because all parent bones are static from the
         * solver's point of view.
         */
        ik_vec3_copy(target, eff->target_position.f);
        ik_transform_bone_pos_g2l(target, ik_bone_get_parent(root_bone), ik_bone_get_parent(base_bone));

        /* In order to lerp between tip bone position and target, transform tip
         * bone position into same space */
        tip_pos = tip_bone->position;
        ik_transform_bone_pos_l2g(tip_pos.f, tip_bone, ik_bone_get_parent(base_bone));

        /* lerp by effector weight to get actual target position */
        ik_vec3_sub_vec3(target, tip_pos.f);
        ik_vec3_mul_scalar(target, eff->weight);
        ik_vec3_add_vec3(target, tip_pos.f);

        /* nlerp actual target position around the next sub-base bone. Makes
         * transitions look more natural */
        if (eff->features & IK_EFFECTOR_WEIGHT_NLERP)
        {
            ikreal distance_to_target;
            const struct ik_bone* subbase_bone = chain_get_base_bone(eff_chain);
            union ik_vec3 to_tip = tip_bone->position;
            union ik_vec3 to_eff = eff->target_position;

            /* Need two vectors from subbase to tip and from subbase to effector target */
            ik_transform_bone_pos_l2g(to_tip.f, tip_bone, subbase_bone);
            ik_transform_bone_pos_g2l(to_eff.f, ik_bone_get_parent(root_bone), subbase_bone);

            /* The effective distance is a lerp between the distances of these two vectors*/
            distance_to_target = ik_vec3_length(to_eff.f) * eff->weight;
            distance_to_target += ik_vec3_length(to_tip.f) * (1.0 - eff->weight);

            /* nlerp the target position by pinning it to the base bone */
            ik_transform_bone_pos_g2l(target, ik_bone_get_parent(base_bone), subbase_bone);
            ik_vec3_normalize(target);
            ik_vec3_mul_scalar(target, distance_to_target);
            ik_transform_bone_pos_l2g(target, subbase_bone, ik_bone_get_parent(base_bone));
        }
    }
}

/* ------------------------------------------------------------------------- */
static void
transform_target_to_local_space(ikreal target[3],
                                const struct ik_bone* root,
                                const struct ik_bone* bone)
{
    const struct ik_bone* parent = ik_bone_get_parent(bone);
    if (parent != root)
        transform_target_to_local_space(target, root, parent);

    ik_vec3_sub_vec3(target, parent->position.f);
    ik_vec3_rotate_quat_conj(target, bone->rotation.f);
}
static union ik_vec3
solve_chain_forwards_recurse(struct ik_chain* chain,
                             union ik_vec3** target_store,
                             const struct ik_bone* root)
{
    union ik_vec3 target;
    int avg_count;

    /* Target position for the tip of each chain is the average position of all
     * solved base bone positions */
    avg_count = 0;
    ik_vec3_set_zero(target.f);
    CHAIN_FOR_EACH_CHILD(chain, child)
        union ik_vec3 base_pos = solve_chain_forwards_recurse(child, target_store, root);
        ik_vec3_add_vec3(target.f, base_pos.f);
        ++avg_count;
    CHAIN_END_EACH

    if (avg_count == 0)
    {
        target = *(*target_store)++;
        transform_target_to_local_space(target.f, root, chain_get_tip_bone(chain));
    }
    else
        ik_vec3_div_scalar(target.f, avg_count);

    CHAIN_FOR_EACH_BONE(chain, bone)

    CHAIN_END_EACH

    return target;
}
static union ik_vec3
solve_chain_forwards(struct ik_solver_fabrik* solver)
{
    const struct ik_bone* base_bone = chain_get_base_bone(&solver->chain_tree);
    union ik_vec3* target_store = solver->target_positions;
    union ik_vec3 target = solve_chain_forwards_recurse(&solver->chain_tree, &target_store, base_bone);

    /* This sets up the target position correctly for backwards iteration */
    ik_vec3_sub_vec3(target.f, base_bone->position.f);
    ik_vec3_negate(target.f);
    ik_vec3_add_vec3(target.f, base_bone->position.f);

    return target;
}

/* ------------------------------------------------------------------------- */
static void
solve_chain_backwards_constraints_recurse(struct ik_chain* chain, union ik_vec3 target)
{
    CHAIN_FOR_EACH_BONE_R(chain, bone)

    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD(chain, child)
        solve_chain_backwards_constraints_recurse(child, target);
    CHAIN_END_EACH
}
static void
solve_chain_backwards_constraints(struct ik_solver_fabrik* solver, union ik_vec3 target)
{
    solve_chain_backwards_constraints_recurse(&solver->chain_tree, target);
}

/* ------------------------------------------------------------------------- */
static void
solve_chain_backwards_recurse(struct ik_chain* chain, union ik_vec3 target)
{
    CHAIN_FOR_EACH_BONE(chain, bone)

    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD(chain, child)
        solve_chain_backwards_recurse(child, target);
    CHAIN_END_EACH
}
static void
solve_chain_backwards(struct ik_solver_fabrik* solver, union ik_vec3 target)
{
    solve_chain_backwards_recurse(&solver->chain_tree, target);
}

/* ------------------------------------------------------------------------- */
static int
fabrik_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    int num_chains;
    void* buf;
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    chain_tree_init(&solver->chain_tree);
    if (chain_tree_build(&solver->chain_tree, subtree) != 0)
        goto build_chain_tree_failed;

    solver->num_effectors = subtree_leaves(subtree);
    num_chains = count_chains(&solver->chain_tree);

    solver->effector_chains = MALLOC(sizeof(*solver->effector_chains) * solver->num_effectors);
    if (solver->effector_chains == NULL)
        goto alloc_effector_bones_failed;

    solver->target_positions = MALLOC(sizeof(*solver->target_positions) * solver->num_effectors);
    if (solver->target_positions == NULL)
        goto alloc_target_positions_failed;

    store_effector_chains(solver);
    validate_poles(solver);

    ik_log_printf(IK_DEBUG, "FABRIK: Initialized with %d end-effectors. %d chains were created.",
                  solver->num_effectors, num_chains);

    return 0;

    alloc_target_positions_failed   : FREE(solver->effector_bones);
    alloc_effector_bones_failed     :
    build_chain_tree_failed         : chain_tree_deinit(&solver->chain_tree);
    return -1;
}

/* ------------------------------------------------------------------------- */
static void
fabrik_deinit(struct ik_solver* solver_base)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    chain_tree_deinit(&solver->chain_tree);
    FREE(solver->effector_bones);
}

/* ------------------------------------------------------------------------- */
static int
all_targets_reached(struct ik_solver_fabrik* solver, ikreal tol_squared)
{
    /* TODO broken
    int i;
    for (i = 0; i != solver->num_effectors; ++i)
    {
        struct ik_bone* bone = solver->effector_bones[i];
        struct ik_effector* eff = bone->effector;
        union ik_vec3 diff = bone->position;

        ik_vec3_sub_vec3(diff.f, eff->target_position.f);
        if (ik_vec3_length_squared(diff.f) > tol_squared)
            return 0;
    }*/

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
fabrik_solve(struct ik_solver* solver_base)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;
    struct ik_algorithm* alg = solver->algorithm;
    int iteration = alg->max_iterations;
    ikreal tol_squared = alg->tolerance * alg->tolerance;

    calculate_target_data(solver);

    while (iteration-- > 0)
    {
        union ik_vec3 base_pos = solve_chain_forwards(solver);
        //solve_chain_backwards(solver, base_pos);

        /*if (alg->features & IK_ALGORITHM_CONSTRAINTS)
            solve_chain_backwards_constraints(solver, base_pos);
        else*/

        if (all_targets_reached(solver, tol_squared))
            break;
    }

    return alg->max_iterations - iteration;
}

/* ------------------------------------------------------------------------- */
static void
visit_bones_recursive(const struct ik_chain* chain, ik_visit_bone_func visit, void* param)
{
    CHAIN_FOR_EACH_BONE_R(chain, bone)
        visit(bone, param);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_CHILD(chain, child)
        visit_bones_recursive(child, visit, param);
    CHAIN_END_EACH
}
static void
fabrik_visit_bones(const struct ik_solver* solver_base, ik_visit_bone_func visit, void* param)
{
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;
    visit_bones_recursive(&solver->chain_tree, visit, param);
}

/* ------------------------------------------------------------------------- */
static void
fabrik_visit_effectors(const struct ik_solver* solver_base, ik_visit_bone_func visit, void* param)
{
    int i;
    struct ik_solver_fabrik* solver = (struct ik_solver_fabrik*)solver_base;

    for (i = 0; i != solver->num_effectors; ++i)
        visit(solver->effector_bones[i], param);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_FABRIK = {
    "fabrik",
    sizeof(struct ik_solver_fabrik),
    fabrik_init,
    fabrik_deinit,
    fabrik_solve,
    fabrik_visit_bones,
    fabrik_visit_effectors
};
