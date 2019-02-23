#include "ik/solver_update.h"
#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/node_data.h"
#include "ik/vec3.h"
#include "ik/vector.h"

/* ------------------------------------------------------------------------- */
static void
update_effector_target(struct ik_chain_t* chain)
{
    struct ik_node_data_t* tip = chain->effector_node;
    struct ik_node_data_t* base = chain->base_node;
    struct ik_effector_t* effector = chain->effector_node->effector;

    /* lerp using effector weight to get actual target position */
    effector->actual_target = effector->target_position;
    ik_vec3_sub_vec3(effector->actual_target.f, tip->transform.t.position.f);
    ik_vec3_mul_scalar(effector->actual_target.f, effector->weight);
    ik_vec3_add_vec3(effector->actual_target.f, tip->transform.t.position.f);

    /* Fancy algorithm using nlerp, makes transitions look more natural */
    if (effector->features & IK_EFFECTOR_WEIGHT_NLERP && effector->weight < 1.0)
    {
        ikreal_t distance_to_target;
        union ik_vec3_t base_to_effector;
        union ik_vec3_t base_to_target;

        /* Need distance from base node to target and base to effector node */
        base_to_effector = tip->transform.t.position;
        base_to_target = effector->target_position;
        ik_vec3_sub_vec3(base_to_effector.f, base->transform.t.position.f);
        ik_vec3_sub_vec3(base_to_target.f, base->transform.t.position.f);

        /* The effective distance is a lerp between these two distances */
        distance_to_target = ik_vec3_length(base_to_target.f) * effector->weight;
        distance_to_target += ik_vec3_length(base_to_effector.f) * (1.0 - effector->weight);

        /* nlerp the target position by pinning it to the base node */
        ik_vec3_sub_vec3(effector->actual_target.f, base->transform.t.position.f);
        ik_vec3_normalize(effector->actual_target.f);
        ik_vec3_mul_scalar(effector->actual_target.f, distance_to_target);
        ik_vec3_add_vec3(effector->actual_target.f, base->transform.t.position.f);
    }
}

/* ------------------------------------------------------------------------- */
void
ik_solver_update_effector_targets(struct vector_t* effector_chains)
{
    VECTOR_FOR_EACH(effector_chains, struct ik_chain_t, chain)
        update_effector_target(chain);
    VECTOR_END_EACH
}

/* ------------------------------------------------------------------------- */
void
ik_solver_update_node_distances(struct vector_t* ntf_list)
{
}
