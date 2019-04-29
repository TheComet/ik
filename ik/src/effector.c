#include "cstructures/memory.h"
#include "ik/effector.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/node_data.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include <string.h>
#include <assert.h>

#define IK_FAIL(errcode, label) do { \
        status = errcode; \
        goto label; \
    } while (0)

static void
deinit_effector(struct ik_effector_t* effector)
{
    /* No data is managed by constraint */
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_effector_create(struct ik_effector_t** effector)
{
    ikret_t status;

    *effector = MALLOC(sizeof **effector);
    if (effector == NULL)
    {
        ik_log_fatal("Failed to allocate effector: Not enough memory");
        IK_FAIL(IK_ERR_OUT_OF_MEMORY, alloc_effector_failed);
    }

    memset(*effector, 0, sizeof **effector);

    if ((status = ik_refcount_create(&(*effector)->refcount,
            (ik_deinit_func)deinit_effector, 1)) != IK_OK)
        IK_FAIL(status, init_refcount_failed);

    ik_vec3_set_zero((*effector)->target_position.f);
    ik_quat_set_identity((*effector)->target_rotation.f);
    (*effector)->weight = 1.0;
    (*effector)->rotation_weight = 1.0;
    (*effector)->rotation_decay = 0.25;

    return IK_OK;

    init_refcount_failed  : FREE(*effector);
    alloc_effector_failed : return status;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_free(struct ik_effector_t* effector)
{
    IK_DECREF(effector);
}

/* ------------------------------------------------------------------------- */
void
ik_effector_set_target_position(struct ik_effector_t* eff, const ikreal_t pos[3])
{
    ik_vec3_copy(eff->target_position.f, pos);
}

/* ------------------------------------------------------------------------- */
const ikreal_t*
ik_effector_get_target_position(const struct ik_effector_t* eff)
{
    return eff->target_position.f;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_set_target_rotation(struct ik_effector_t* eff, const ikreal_t rot[4])
{
    ik_quat_copy(eff->target_rotation.f, rot);
}

/* ------------------------------------------------------------------------- */
const ikreal_t*
ik_effector_get_target_rotation(const struct ik_effector_t* eff)
{
    return eff->target_rotation.f;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_set_weight(struct ik_effector_t* eff, ikreal_t weight)
{
    eff->weight = weight;
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_effector_get_weight(const struct ik_effector_t* eff)
{
    return eff->weight;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_set_rotation_weight(struct ik_effector_t* eff, ikreal_t weight)
{
    eff->rotation_weight = weight;
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_effector_get_rotation_weight(const struct ik_effector_t* eff)
{
    return eff->rotation_weight;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_set_rotation_weight_decay(struct ik_effector_t* eff, ikreal_t decay)
{
    eff->rotation_decay = decay;
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_effector_get_rotation_weight_decay(const struct ik_effector_t* eff)
{
    return eff->rotation_decay;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_set_chain_length(struct ik_effector_t* eff, uint16_t length)
{
    eff->chain_length = length;
}

/* ------------------------------------------------------------------------- */
uint16_t
ik_effector_get_chain_length(const struct ik_effector_t* eff)
{
    return eff->chain_length;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_enable_features(struct ik_effector_t* eff, uint8_t features)
{
    eff->features |= features;
}

/* ------------------------------------------------------------------------- */
void
ik_effector_disable_features(struct ik_effector_t* eff, uint8_t features)
{
    eff->features &= ~features;
}

/* ------------------------------------------------------------------------- */
uint8_t
ik_effector_get_features(const struct ik_effector_t* eff)
{
    return eff->features;
}

/* ------------------------------------------------------------------------- */
uint8_t
ik_effector_is_feature_enabled(const struct ik_effector_t* eff, enum ik_effector_features_e feature)
{
    return (eff->features & feature) == feature;
}
