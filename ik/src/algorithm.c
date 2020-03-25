#include "ik/algorithm.h"
#include "ik/log.h"
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
ik_algorithm_create(const char* name)
{
    struct ik_algorithm* alg;

    uintptr_t len = strlen(name);
    if (len >= sizeof(((struct ik_algorithm*)0)->name))
    {
        ik_log_printf(IK_ERROR, "Algorithm name `%s` is too long.", name);
        return NULL;
    }
    if (len == 0)
    {
        ik_log_printf(IK_ERROR, "Empty algorithm name provided.");
        return NULL;
    }

    alg = (struct ik_algorithm*)
        ik_attachment_alloc(sizeof *alg, (ik_deinit_func)deinit_algorithm);
    if (alg == NULL)
        return NULL;

    strcpy(alg->name, name);
    alg->tolerance = 1e-2;
    alg->max_iterations = 20;
    alg->features = IK_SOLVER_JOINT_ROTATIONS;

    return alg;
}
