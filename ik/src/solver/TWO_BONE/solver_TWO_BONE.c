#include "ik/solver_TWO_BONE.h"
#include "ik/chain.h"
#include "ik/ik.h"
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
ik_solver_TWO_BONE_rebuild_data(struct ik_solver_t* solver)
{
    /*
     * We need to assert that there really are only chains of length 1 and no
     * sub chains.
     */
    SOLVER_FOR_EACH_CHAIN(solver, chain)
        if (chain_length(chain) != 3) /* 3 nodes = 2 bones */
        {
            ik.log.message("ERROR: Your tree has chains that are longer or shorter than 2 bones. Are you sure you selected the correct solver algorithm?");
            return -1;
        }
        if (chain_length(chain) > 0)
        {
            ik.log.message("ERROR: Your tree has child chains. This solver does not support arbitrary trees. You will need to switch to another algorithm (e.g. FABRIK)");
            return -1;
        }
    SOLVER_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_TWO_BONE_solve(struct ik_solver_t* solver)
{
    SOLVER_FOR_EACH_CHAIN(solver, chain)
        struct ik_node_t* node_tip;
        struct ik_node_t* node_mid;
        struct ik_node_t* node_base;
        vec3_t to_target;
        ik_real a, b, c, aa, bb, cc;

        assert(chain_length(chain) > 2);
        node_tip  = chain_get_node(chain, 0);
        node_mid  = chain_get_node(chain, 1);
        node_base = chain_get_node(chain, 2);

        assert(node_tip->effector != NULL);
        to_target = node_tip->effector->target_position;
        vec3_sub_vec3(to_target.f, node_base->position.f);

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
        cc = vec3_length_squared(to_target.f);
        c = sqrt(cc);

        /* check if in reach */
        if (c < a + b)
        {
            /* Cosine law to get base angle (alpha) */
            quat_t alpha_rotation;
            ik_real alpha = acos((bb + cc - aa) / (2.0 * node_mid->dist_to_parent * sqrt(cc)));
            ik_real cos_a = cos(alpha * 0.5);
            ik_real sin_a = sin(alpha * 0.5);

            /* Cross product of both segment vectors defines axis of rotation */
            alpha_rotation.v = node_tip->position;
            vec3_sub_vec3(alpha_rotation.f, node_mid->position.f);  /* top segment */
            vec3_sub_vec3(node_mid->position.f, node_base->position.f);  /* bottom segment */
            vec3_cross(alpha_rotation.f, node_mid->position.f);

            /*
             * Set up quaternion describing the rotation of alpha. Need to
             * normalise vec3 component of quaternion so rotation is correct.
             */
            vec3_normalize(alpha_rotation.f);
            vec3_mul_scalar(alpha_rotation.f, sin_a);
            alpha_rotation.w = cos_a;

            /* Rotate side c and scale to length of side b to get the unknown position */
            node_mid->position = to_target;
            vec3_normalize(node_mid->position.f);
            vec3_mul_scalar(node_mid->position.f, node_mid->dist_to_parent);
            quat_rotate_vec(node_mid->position.f, alpha_rotation.f);
            vec3_add_vec3(node_mid->position.f, node_base->position.f);

            node_tip->position = node_tip->effector->target_position;
        }
        else
        {
            /* Just point both segments at target */
            vec3_normalize(to_target.f);
            node_mid->position = to_target;
            node_tip->position = to_target;
            vec3_mul_scalar(node_mid->position.f, node_mid->dist_to_parent);
            vec3_mul_scalar(node_tip->position.f, node_tip->dist_to_parent);
            vec3_add_vec3(node_mid->position.f, node_base->position.f);
            vec3_add_vec3(node_tip->position.f, node_mid->position.f);
        }
    SOLVER_END_EACH

    return 0;
}
