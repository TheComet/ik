#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/ntf.h"
#include "ik/node_data.h"
#include "ik/solver_prepare.h"
#include "ik/solver.h"
#include "ik/transform.h"
#include "ik/vector.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_solver_prepare_stack_buffer(struct ik_solver_t* solver)
{
    return IK_OK;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_prepare_pole_targets(struct ik_solver_t* solver)
{
}
