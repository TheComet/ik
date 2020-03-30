#include "ik/python/ik_type_Attachment.h"

#define ik_constraint_CheckExact(o) \
    (Py_TYPE(o) == &ik_ConstraintType)

struct ik_constraint;

typedef struct ik_Constraint
{
    ik_Attachment super;
} ik_Constraint;

extern PyTypeObject ik_ConstraintType;

int
init_ik_ConstraintType(void);
