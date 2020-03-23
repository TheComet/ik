#include "ik/algorithm.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include <string.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
static void
deinit_algorithm(struct ik_algorithm* alg)
{

}

/* ------------------------------------------------------------------------- */
struct ik_algorithm*
ik_algorithm_create(void)
{
    struct ik_algorithm* alg = (struct ik_algorithm*)
        ik_attachment_alloc(sizeof *alg, (ik_deinit_func)deinit_algorithm);
    if (alg == NULL)
        return NULL;

    alg->tolerance = 1e-2;
    alg->max_iterations = 20;
    alg->features = IK_SOLVER_JOINT_ROTATIONS;
    alg->algorithm = IK_SOLVER_FABRIK;

    return IK_OK;
}
