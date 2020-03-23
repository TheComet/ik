#include "ik/effector.h"
#include <stddef.h>
#include <stdio.h>

/* ------------------------------------------------------------------------- */
static void
deinit_effector(struct ik_effector* eff)
{
}

/* ------------------------------------------------------------------------- */
struct ik_effector*
ik_effector_create(void)
{
    struct ik_effector* eff = (struct ik_effector*)
        ik_attachment_alloc(sizeof *eff, (ik_deinit_func)deinit_effector);
    if (eff == NULL)
        return NULL;

    ik_vec3_set_zero(eff->target_position.f);
    ik_quat_set_identity(eff->target_rotation.f);
    eff->weight = 1.0;
    eff->rotation_weight = 1.0;
    eff->rotation_decay = 0.25;
    eff->chain_length = 0;
    eff->features = 0;

    return eff;
}
