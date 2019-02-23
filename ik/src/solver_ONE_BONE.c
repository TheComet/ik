#include "ik/effector.h"
#include "ik/ntf.h"
#include "ik/log.h"
#include "ik/node_data.h"
#include "ik/solver_head.h"
#include "ik/solver_ONE_BONE.h"
#include "ik/vec3.h"
#include <stddef.h>
#include <assert.h>

struct ik_solver_t
{
    IK_SOLVER_HEAD
};

/* ------------------------------------------------------------------------- */
uintptr_t
ik_solver_ONE_BONE_type_size(void)
{
    return sizeof(struct ik_solver_t);
}

/* ------------------------------------------------------------------------- */
int
ik_solver_ONE_BONE_construct(struct ik_solver_t* solver)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_ONE_BONE_destruct(struct ik_solver_t* solver)
{
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_ONE_BONE_prepare(struct ik_solver_t* solver)
{
    /*
     * We need to assert that there really are only chains of length 1 and no
     * sub chains.
     */
    NTF_FOR_EACH(&solver->ntf_list, ntf)
        uint32_t i = ntf->node_count;
        while (i--)
        {
            if (ntf->indices[i].post_child_count > 1)
            {
                ik_log_error("Your tree has child chains. This solver does not support multiple end effectors. You will need to switch to another algorithm (e.g. FABRIK)");
                return IK_ERR_GENERIC;
            }
        }

        if (ntf->node_count != 2) /* 2 nodes = 1 bone */
        {
            ik_log_error("Your tree has chains that are longer than 1 bone. Are you sure you selected the correct solver algorithm?");
            return IK_ERR_GENERIC;
        }
    NTF_END_EACH

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_ONE_BONE_solve(struct ik_solver_t* solver)
{
    NTF_FOR_EACH(&solver->ntf_list, ntf)
        struct ik_node_data_t* tip;
        struct ik_node_data_t* base;
        ikreal_t* tip_pos;
        ikreal_t* base_pos;
        ikreal_t* target_pos;

        assert(ntf->node_count == 2);
        tip  = NTF_GET_PRE(ntf, 0);
        base = NTF_GET_PRE(ntf, 1);

        assert(tip->effector != NULL);
        tip_pos = tip->transform.t.position.f;
        base_pos = base->transform.t.position.f;
        target_pos = tip->effector->target_position.f;

        ik_vec3_copy(tip_pos, target_pos);
        ik_vec3_sub_vec3(tip_pos, base_pos);
        ik_vec3_normalize(tip_pos);
        ik_vec3_mul_scalar(tip_pos, tip->dist_to_parent);
        ik_vec3_add_vec3(tip_pos, base_pos);
    NTF_END_EACH

    return 0;
}
