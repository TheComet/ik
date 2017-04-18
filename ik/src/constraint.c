#include "ik/constraint.h"
#include "ik/log.h"
#include "ik/memory.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
ik_constraint_t*
ik_constraint_create(void)
{
    ik_constraint_t* constraint = (ik_constraint_t*)MALLOC(sizeof *constraint);
    if (constraint == NULL)
    {
        ik_log_message("Failed to allocate constraint: Out of memory");
        return NULL;
    }
    ik_constraint_construct(constraint);
    return constraint;
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_construct(ik_constraint_t* constraint)
{
    memset(constraint, 0, sizeof *constraint);
}

/* ------------------------------------------------------------------------- */
void
ik_constraint_destroy(ik_constraint_t* constraint)
{
    FREE(constraint);
}
