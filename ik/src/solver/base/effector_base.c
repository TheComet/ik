#include "ik/ik.h"
#include "ik/memory.h"
#include "ik/impl/effector_base.h"
#include "ik/impl/log.h"
#include "ik/impl/quat.h"
#include "ik/impl/vec3.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
struct ik_effector_t*
ik_effector_base_create(void)
{
    struct ik_effector_t* effector = MALLOC(sizeof *effector);
    if (effector == NULL)
        return NULL;

    memset(effector, 0, sizeof *effector);
    ik_vec3_set_zero(effector->target_position.f);
    ik_quat_set_identity(effector->target_rotation.f);
    effector->weight = 1.0;
    effector->rotation_weight = 1.0;
    effector->rotation_decay = 0.25;
    effector->v = &IKAPI.base.effector_base;

    return effector;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_base_destroy(struct ik_effector_t* effector)
{
    ik_effector_base_detach(effector);
    FREE(effector);
}

/* ------------------------------------------------------------------------- */
struct ik_effector_t*
ik_effector_base_duplicate(const struct ik_effector_t* effector)
{
    struct ik_effector_t* new_effector = effector->v->create();
    if (effector == NULL)
        return NULL;

    new_effector->node = NULL;
    new_effector->target_position = effector->target_position;
    new_effector->target_rotation = effector->target_rotation;
    new_effector->_actual_target  = effector->_actual_target;
    new_effector->weight          = effector->weight;
    new_effector->rotation_weight = effector->rotation_weight;
    new_effector->rotation_decay  = effector->rotation_decay;
    new_effector->chain_length    = effector->chain_length;
    new_effector->flags           = effector->flags;

    return new_effector;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_effector_base_attach(struct ik_effector_t* effector, struct ik_node_t* node)
{
    if (node->effector != NULL)
    {
        ik_log_error(
            "You are trying to attach an effector to a node that "
            "already has an effector attached to it. The new effector will not "
            "be attached!"
        );
        return IK_ALREADY_HAS_ATTACHMENT;
    }

    /* effector may be attached to another node */
    effector->v->detach(effector);

    node->effector = effector;
    effector->node = node;

    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_base_detach(struct ik_effector_t* effector)
{
    if (effector->node == NULL)
        return;

    effector->node->effector = NULL;
    effector->node = NULL;
}
