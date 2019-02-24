#include "ik/effector.h"
#include "ik/ntf.h"
#include "ik/log.h"
#include "ik/node_data.h"
#include "ik/algorithm_head.h"
#include "ik/algorithm_b1.h"
#include "ik/vec3.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
uintptr_t
ik_algorithm_b1_type_size(void)
{
    return sizeof(struct ik_algorithm_b1_t);
}

/* ------------------------------------------------------------------------- */
int
ik_algorithm_b1_construct(struct ik_algorithm_b1_t* algorithm)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_b1_destruct(struct ik_algorithm_b1_t* algorithm)
{
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_algorithm_b1_prepare(struct ik_algorithm_b1_t* algorithm)
{
    /*
     * We need to assert that there really are only chains of length 1 and no
     * sub chains.
     */
    uint32_t i = algorithm->ntf->node_count;
    while (i--)
    {
        if (NTF_POST_CHILD_COUNT(algorithm->ntf, i) > 1)
        {
            ik_log_error("Your tree has child chains. This algorithm does not support multiple end effectors. You will need to switch to another algorithm (e.g. FABRIK)");
            return IK_ERR_GENERIC;
        }
    }

    if (algorithm->ntf->node_count != 2) /* 2 nodes = 1 bone */
    {
        ik_log_error("Your tree has chains that are longer than 1 bone. Are you sure you selected the correct algorithm algorithm?");
        return IK_ERR_GENERIC;
    }

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
int
ik_algorithm_b1_solve(struct ik_algorithm_b1_t* algorithm)
{
    struct ik_node_data_t* tip;
    struct ik_node_data_t* base;
    struct ik_effector_t* eff;
    ikreal_t* tip_pos;
    ikreal_t* base_pos;
    ikreal_t* target_pos;

    assert(algorithm->ntf->node_count == 2);
    tip  = NTF_PRE_NODE(algorithm->ntf, 0);
    base = NTF_PRE_NODE(algorithm->ntf, 1);
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
