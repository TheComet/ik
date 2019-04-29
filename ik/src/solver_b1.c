#include "ik/effector.h"
#include "ik/ntf.h"
#include "ik/log.h"
#include "ik/node_data.h"
#include "ik/solver_head.h"
#include "ik/solver_b1.h"
#include "ik/vec3.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
uintptr_t
ik_solver_b1_type_size(void)
{
    return sizeof(struct ik_solver_b1_t);
}

/* ------------------------------------------------------------------------- */
int
ik_solver_b1_init(struct ik_solver_b1_t* solver)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_b1_deinit(struct ik_solver_b1_t* solver)
{
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_b1_prepare(struct ik_solver_b1_t* solver)
{
    /*
     * We need to assert that there really are only chains of length 1 and no
     * sub chains.
     */
    uint32_t i = solver->ntf->node_count;
    while (i--)
    {
        if (NTF_POST_CHILD_COUNT(solver->ntf, i) > 1)
        {
            ik_log_error("Your tree has child chains. This solver does not support multiple end effectors. You will need to switch to another solver (e.g. FABRIK)");
            return IK_ERR_GENERIC;
        }
    }

    if (solver->ntf->node_count != 2) /* 2 nodes = 1 bone */
    {
        ik_log_error("Your tree has chains that are longer than 1 bone. Are you sure you selected the correct solver solver?");
        return IK_ERR_GENERIC;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_b1_solve(struct ik_solver_b1_t* solver)
{
    struct ik_node_data_t* tip;
    struct ik_node_data_t* base;
    struct ik_effector_t* eff;
    ikreal_t* tip_pos;
    ikreal_t* base_pos;
    ikreal_t* target_pos;

    assert(solver->ntf->node_count == 2);
    tip  = NTF_PRE_NODE(solver->ntf, 0);
    base = NTF_PRE_NODE(solver->ntf, 1);
    eff = (struct ik_effector_t*)tip->attachment[IK_ATTACHMENT_EFFECTOR];

    assert(tip->attachment[IK_ATTACHMENT_EFFECTOR] != NULL);
    tip_pos    = tip->transform.t.position.f;
    base_pos   = base->transform.t.position.f;
    target_pos = eff->target_position.f;

    ik_vec3_copy(tip_pos, target_pos);
    ik_vec3_sub_vec3(tip_pos, base_pos);
    ik_vec3_normalize(tip_pos);
    ik_vec3_mul_scalar(tip_pos, tip->dist_to_parent);
    ik_vec3_add_vec3(tip_pos, base_pos);

    return 0;
}
