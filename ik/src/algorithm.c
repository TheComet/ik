#include "ik/algorithm.h"
#include "ik/log.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include <string.h>
#include <assert.h>

static int
is_algorithm_type_valid(const char* type)
{
    uintptr_t len = strlen(type);
    if (len >= sizeof(((struct ik_algorithm*)0)->type))
    {
        ik_log_printf(IK_ERROR, "Algorithm type `%s` is too long.", type);
        return 0;
    }
    if (len == 0)
    {
        ik_log_printf(IK_ERROR, "Empty algorithm type provided.");
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------------- */
static void
deinit_algorithm(struct ik_algorithm* alg)
{
}

/* ------------------------------------------------------------------------- */
struct ik_algorithm*
ik_algorithm_create(const char* type)
{
    struct ik_algorithm* alg;

    if (!is_algorithm_type_valid(type))
        return NULL;

    alg = (struct ik_algorithm*)
        ik_attachment_alloc(sizeof *alg, (ik_deinit_func)deinit_algorithm);
    if (alg == NULL)
        return NULL;

    strcpy(alg->type, type);
    alg->tolerance = 1e-2;
    alg->max_iterations = 20;
    alg->features = 0;

    return alg;
}

/* ------------------------------------------------------------------------- */
int
ik_algorithm_set_type(struct ik_algorithm* algorithm, const char* type)
{
    if (!is_algorithm_type_valid(type))
        return -1;

    strcpy(algorithm->type, type);
    return 0;
}
