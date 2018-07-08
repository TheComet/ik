#include "ik/effector_base.h"
#include "ik/ik.h"
#include "ik/memory.h"
#include "ik/vec3_static.h"
#include "ik/quat_static.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
struct ik_effector_t*
ik_effector_base_create(void)
{
    struct ik_effector_t* effector = MALLOC(sizeof *effector);
    if (effector == NULL)
        return NULL;

    memset(effector, 0, sizeof *effector);
    ik_vec3_static_set_zero(effector->target_position.f);
    ik_quat_static_set_identity(effector->target_rotation.f);
    effector->weight = 1.0;
    effector->rotation_weight = 1.0;
    effector->rotation_decay = 0.25;
    effector->v = &IKAPI.internal.effector_base;

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
int
ik_effector_base_attach(struct ik_effector_t* effector, struct ik_node_t* node)
{
    if (node->effector != NULL)
    {
        IKAPI.log.message(
            "Warning! You are trying to attach an effector to a node that "
            "already has an effector attached to it. The new effector will not "
            "be attached!"
        );
        return -1;
    }

    /* effector may be attached to another node */
    ik_effector_base_detach(effector);

    node->effector = effector;
    effector->node = node;

    return 0;
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
