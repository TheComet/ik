#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/solver_TWO_BONE.h"
#include "ik/transform.h"
#include "ik/vec3.h"
#include <assert.h>
#include <math.h>
#include <stddef.h>

/* ------------------------------------------------------------------------- */
uintptr_t
ik_solver_TWO_BONE_type_size(void)
{
    return sizeof(struct ik_solver_t);
}

/* ------------------------------------------------------------------------- */
int
ik_solver_TWO_BONE_construct(struct ik_solver_t* solver)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_TWO_BONE_destruct(struct ik_solver_t* solver)
{
}

/* ------------------------------------------------------------------------- */
int
ik_solver_TWO_BONE_rebuild(struct ik_solver_t* solver)
{
    /*
     * We need to assert that there really are only chains of length 1 and no
     * sub chains.
     */
    SOLVER_FOR_EACH_CHAIN(solver, chain)
        if (chain_length(chain) != 3) /* 3 nodes = 2 bones */
        {
            ik_log_error("Your tree has chains that are longer or shorter than 2 bones. Are you sure you selected the correct solver algorithm?");
            return -1;
        }
        if (chain_length(chain) > 0)
        {
            ik_log_error("Your tree has child chains. This solver does not support arbitrary trees. You will need to switch to another algorithm (e.g. FABRIK)");
            return -1;
        }
    SOLVER_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_TWO_BONE_solve(struct ik_solver_t* solver)
{
    /* Tree is in local space, we need global positions */
    ik_transform_chain_list(&solver->chain_list, IK_L2G | IK_TRANSLATIONS);

    SOLVER_FOR_EACH_CHAIN(solver, chain)
        struct ik_node_t* node_tip;
        struct ik_node_t* node_mid;
        struct ik_node_t* node_base;
        struct ik_vec3_t to_target;
        ikreal_t a, b, c, aa, bb, cc;

        assert(chain_length(chain) > 2);
        node_tip  = chain_get_node(chain, 0);
        node_mid  = chain_get_node(chain, 1);
        node_base = chain_get_node(chain, 2);

        assert(node_tip->effector != NULL);
        to_target = node_tip->effector->target_position;
        ik_vec3_sub_vec3(to_target.f, node_base->position.f);

        /*
         * Form a triangle from the two segment lengths so we can calculate the
         * angles. Here's some visual help.
         *
         *   target *--.__  a
         *           \     --.___ (unknown position, needs solving)
         *            \      _-
         *           c \   _-
         *              \-    b
         *            base
         *
         */
        a = node_tip->dist_to_parent;
        b = node_mid->dist_to_parent;
        aa = a*a;
        bb = b*b;
        cc = ik_vec3_length_squared(to_target.f);
        c = sqrt(cc);

        /* check if in reach */
        if (c < a + b)
        {
            /* Cosine law to get base angle (alpha) */
            struct ik_quat_t base_rotation;
            ikreal_t base_angle = acos((bb + cc - aa) / (2.0 * b * c));
            ikreal_t cos_a = cos(base_angle * 0.5);
            ikreal_t sin_a = sin(base_angle * 0.5);

            /* Cross product of both segment vectors defines axis of rotation */
            base_rotation.v = node_tip->position;
            ik_vec3_sub_vec3(base_rotation.f, node_mid->position.f);  /* top segment */
            ik_vec3_sub_vec3(node_mid->position.f, node_base->position.f);  /* bottom segment */
            ik_vec3_cross(base_rotation.f, node_mid->position.f);

            /*
             * Set up quaternion describing the rotation of alpha. Need to
             * normalise vec3 component of quaternion so rotation is correct.
             * NOTE: Normalize will give us (1,0,0) in case of giving it a zero
             * length vector. We rely on this behavior for a default axis.
             */
            ik_vec3_normalize(base_rotation.f);
            ik_vec3_mul_scalar(base_rotation.f, sin_a);
            base_rotation.w = cos_a;

            /*
             * Rotate side c and scale to length of side b to get the unknown
             * position. node_base was already subtracted from node_mid
             * previously, which means it will rotate around the base node's
             * position (as it should)
             */
            node_mid->position = to_target;
            ik_vec3_normalize(node_mid->position.f);
            ik_vec3_rotate(node_mid->position.f, base_rotation.f);
            ik_vec3_mul_scalar(node_mid->position.f, node_mid->dist_to_parent);
            ik_vec3_add_vec3(node_mid->position.f, node_base->position.f);

            node_tip->position = node_tip->effector->target_position;
        }
        else
        {
            /* Just point both segments at target */
            ik_vec3_normalize(to_target.f);
            node_mid->position = to_target;
            node_tip->position = to_target;
            ik_vec3_mul_scalar(node_mid->position.f, node_mid->dist_to_parent);
            ik_vec3_mul_scalar(node_tip->position.f, node_tip->dist_to_parent);
            ik_vec3_add_vec3(node_mid->position.f, node_base->position.f);
            ik_vec3_add_vec3(node_tip->position.f, node_mid->position.f);
        }
    SOLVER_END_EACH

    /* Transform back again */
    ik_transform_chain_list(&solver->chain_list, IK_G2L | IK_TRANSLATIONS);

    return 0;
}
