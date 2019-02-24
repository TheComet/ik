#include "ik/log.h"
#include "ik/memory.h"
#include "ik/ntf.h"
#include "ik/algorithm_head.h"
#include "ik/algorithm.h"
#include "ik/algorithm_prepare.h"
#include "ik/algorithm_update.h"
#include "ik/algorithm_ONE_BONE.h"
#include "ik/algorithm_TWO_BONE.h"
#include "ik/algorithm_FABRIK.h"
#include "ik/algorithm_MSS.h"
#include <assert.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_algorithm_create(struct ik_algorithm_t** algorithm, enum ik_algorithm_type type)
{
    switch (type)
    {
#define X(algo_type)                                                          \
        case IK_ALGORITHM_##algo_type : {                                     \
            *algorithm = MALLOC(ik_algorithm_##algo_type##_type_size());      \
            if (*algorithm == NULL) {                                         \
                ik_log_fatal("Failed to allocate algorithm: ran out of memory"); \
                goto alloc_algorithm_failed;                                  \
            }                                                                 \
            memset(*algorithm, 0, ik_algorithm_##algo_type##_type_size());    \
            (*algorithm)->construct = ik_algorithm_##algo_type##_construct;   \
            (*algorithm)->destruct = ik_algorithm_##algo_type##_destruct;     \
            (*algorithm)->prepare = ik_algorithm_##algo_type##_prepare;       \
            (*algorithm)->solve = ik_algorithm_##algo_type##_solve;           \
        } break;
        IK_ALGORITHM_LIST
#undef X
        default : {
            ik_log_error("Unknown algorithm algorithm with enum value %d", algorithm);
            goto alloc_algorithm_failed;
        } break;
    }

    if (ik_algorithm_construct(*algorithm) != IK_OK)
        goto construct_algorithm_failed;

    return IK_OK;

    construct_algorithm_failed : FREE(*algorithm);
    alloc_algorithm_failed     : return IK_ERR_OUT_OF_MEMORY;
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_destroy(struct ik_algorithm_t* algorithm)
{
    ik_algorithm_destruct(algorithm);
    FREE(algorithm);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_algorithm_construct(struct ik_algorithm_t* algorithm)
{
    algorithm->max_iterations = 20;
    algorithm->tolerance = 1e-2;
    algorithm->features = IK_ALGORITHM_JOINT_ROTATIONS;

    return algorithm->construct(algorithm);
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_destruct(struct ik_algorithm_t* algorithm)
{
    algorithm->destruct(algorithm);
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_algorithm_prepare(struct ik_algorithm_t* algorithm, struct ik_node_t* node)
{
    ikret_t status;

    if ((status = ik_algorithm_prepare_stack_buffer(algorithm)) != IK_OK)
        return status;

    ik_algorithm_prepare_pole_targets(algorithm);
    ik_algorithm_update_node_distances(algorithm);

    return algorithm->prepare(algorithm);
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_update_translations(struct ik_algorithm_t* algorithm)
{
    ik_algorithm_update_node_distances(algorithm);
}

/* ------------------------------------------------------------------------- */
uint32_t
ik_algorithm_solve(struct ik_algorithm_t* algorithm)
{
    ik_algorithm_update_effector_targets(algorithm);
    return algorithm->solve(algorithm);
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_iterate_nodes(const struct ik_algorithm_t* algorithm,
                        ik_algorithm_callback_func callback)
{
}


/* ------------------------------------------------------------------------- */
uint16_t
ik_algorithm_get_max_iterations(const struct ik_algorithm_t* algorithm)
{
    return algorithm->max_iterations;
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_set_max_iterations(struct ik_algorithm_t* algorithm, uint16_t max_iterations)
{
    algorithm->max_iterations = max_iterations;
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_algorithm_get_tolerance(const struct ik_algorithm_t* algorithm)
{
    return algorithm->tolerance;
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_set_tolerance(struct ik_algorithm_t* algorithm, ikreal_t tolerance)
{
    algorithm->tolerance = tolerance;
}

/* ------------------------------------------------------------------------- */
uint16_t
ik_algorithm_get_features(const struct ik_algorithm_t* algorithm)
{
    return algorithm->features;
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_typenable_features(struct ik_algorithm_t* algorithm, uint16_t features)
{
    algorithm->features |= features;
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_disable_features(struct ik_algorithm_t* algorithm, uint16_t features)
{
    algorithm->features &= ~features;
}

/* ------------------------------------------------------------------------- */
uint8_t
ik_algorithm_is_feature_enabled(const struct ik_algorithm_t* algorithm,
                             enum ik_algorithm_features_e feature)
{
    return (algorithm->features & feature) == feature;
}

