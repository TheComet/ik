#pragma once

#include "ik/python/ik_type_Attachment.h"

#define ik_Constraint_CheckExact(o) \
    (Py_TYPE(o) == &ik_ConstraintType)

#define ik_Constraint_Check(o) \
    PyObject_TypeCheck(o, &ik_ConstraintType)

struct ik_constraint;
struct ik_Quat;

typedef struct ik_Constraint
{
    ik_Attachment super;
} ik_Constraint;

typedef struct ik_StiffConstraint
{
    ik_Constraint super;
    struct ik_Quat* rotation;
} ik_StiffConstraint;

typedef struct ik_HingeConstraint
{
    ik_Constraint super;
    struct ik_Vec3* axis;
} ik_HingeConstraint;

extern PyTypeObject ik_ConstraintType;
extern PyTypeObject ik_StiffConstraintType;
extern PyTypeObject ik_HingeConstraintType;

int
init_ik_ConstraintType(void);

int
init_ik_StiffConstraintType(void);

int
init_ik_HingeConstraintType(void);
