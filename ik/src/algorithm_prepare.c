#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/ntf.h"
#include "ik/node_data.h"
#include "ik/algorithm_prepare.h"
#include "ik/algorithm.h"
#include "ik/transform.h"
#include "ik/vector.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_algorithm_prepare_stack_buffer(struct ik_algorithm_t* algorithm)
{
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_algorithm_prepare_pole_targets(struct ik_algorithm_t* algorithm)
{
}
