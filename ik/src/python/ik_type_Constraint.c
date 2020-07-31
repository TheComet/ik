#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_helpers.h"
#include "ik/python/ik_docstrings.h"
#include "ik/constraint.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Constraint_dealloc(PyObject* myself)
{
    ik_Constraint* self = (ik_Constraint*)myself;
    IK_DECREF(self->super.attachment);
    ik_ConstraintType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Constraint_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    (void)kwds;
    ik_Constraint* self;
    struct ik_constraint* constraint;

    /* Allocate internal constraint */
    if ((constraint = ik_constraint_create()) == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate internal constraint");
        goto alloc_constraint_failed;
    }
    IK_INCREF(constraint);

    self = (ik_Constraint*)ik_ConstraintType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    self->super.attachment = (struct ik_attachment*)constraint;
    return (PyObject*)self;

    alloc_self_failed       : IK_DECREF(constraint);
    alloc_constraint_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_ConstraintType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Constraint",
    .tp_basicsize = sizeof(ik_Constraint),
    .tp_dealloc = Constraint_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = IK_CONSTRAINT_DOC,
    .tp_new = Constraint_new
};

/* ------------------------------------------------------------------------- */
static void
StiffConstraint_dealloc(PyObject* myself)
{
    ik_StiffConstraint* self = (ik_StiffConstraint*)myself;

    UNREF_QUAT_DATA(self->rotation);
    Py_DECREF(self->rotation);

    ik_StiffConstraintType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
StiffConstraint_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    (void)kwds;
    ik_StiffConstraint* self;
    ik_Quat* rotation;
    struct ik_constraint* constraint;

    rotation = (ik_Quat*)PyObject_CallObject((PyObject*)&ik_QuatType, NULL);
    if (rotation == NULL)
        goto alloc_rotation_failed;

    self = (ik_StiffConstraint*)ik_StiffConstraintType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    constraint = (struct ik_constraint*)self->super.super.attachment;
    ik_constraint_set_stiff(constraint, 0, 0, 0, 1);

    self->rotation = rotation;
    REF_QUAT_DATA(self->rotation, &constraint->data.stiff.rotation);

    return (PyObject*)self;

    alloc_self_failed            : Py_DECREF(rotation);
    alloc_rotation_failed : return NULL;
}


/* ------------------------------------------------------------------------- */
static int
StiffConstraint_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_StiffConstraint* self = (ik_StiffConstraint*)myself;
    ik_Quat* rotation = NULL;

    static char* kwds_str[] = {
        "rotation",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!", kwds_str,
                                     &ik_QuatType, &rotation))
    {
        return -1;
    }

    if (rotation != NULL)
        ASSIGN_QUAT(self->rotation, rotation);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
StiffConstraint_repr(PyObject* myself)
{
    ik_StiffConstraint* self = (ik_StiffConstraint*)myself;
    return PyUnicode_FromFormat("ik.StiffConstraint(rotation=%R)", self->rotation);
}

/* ------------------------------------------------------------------------- */
static PyObject*
StiffConstraint_str(PyObject* myself)
{
    return StiffConstraint_repr(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
StiffConstraint_getrotation(PyObject* myself, void* closure)
{
    ik_StiffConstraint* self = (ik_StiffConstraint*)myself;
    (void)closure;
    return Py_INCREF(self->rotation), (PyObject*)self->rotation;
}
static int
StiffConstraint_setrotation(PyObject* myself, PyObject* value, void* closure)
{
    ik_StiffConstraint* self = (ik_StiffConstraint*)myself;
    (void)closure;
    if (!ik_Quat_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Quat() type");
        return -1;
    }

    ASSIGN_QUAT(self->rotation, (ik_Quat*)value);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef StiffConstraint_getsetters[] = {
    {"rotation", StiffConstraint_getrotation, StiffConstraint_setrotation, IK_STIFFCONSTRAINT_TARGET_ROTATION_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject ik_StiffConstraintType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.StiffConstraint",
    .tp_basicsize = sizeof(ik_StiffConstraint),
    .tp_dealloc = StiffConstraint_dealloc,
    .tp_base = &ik_ConstraintType,
    .tp_repr = StiffConstraint_repr,
    .tp_str = StiffConstraint_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_STIFFCONSTRAINT_DOC,
    .tp_getset = StiffConstraint_getsetters,
    .tp_new = StiffConstraint_new,
    .tp_init = StiffConstraint_init
};

/* ------------------------------------------------------------------------- */
static void
HingeConstraint_dealloc(PyObject* myself)
{
    ik_HingeConstraint* self = (ik_HingeConstraint*)myself;

    UNREF_VEC3_DATA(self->axis);
    Py_DECREF(self->axis);

    ik_HingeConstraintType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
HingeConstraint_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    (void)kwds;
    ik_HingeConstraint* self;
    ik_Vec3* axis;
    struct ik_constraint* constraint;

    axis = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL);
    if (axis == NULL)
        goto alloc_axis_failed;

    self = (ik_HingeConstraint*)ik_HingeConstraintType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    constraint = (struct ik_constraint*)self->super.super.attachment;
    ik_constraint_set_hinge(constraint, 0, 0, 0, 0, 0);

    self->axis = axis;
    REF_VEC3_DATA(self->axis, &constraint->data.hinge.axis);

    return (PyObject*)self;

    alloc_self_failed            : Py_DECREF(axis);
    alloc_axis_failed : return NULL;
}


/* ------------------------------------------------------------------------- */
static int
HingeConstraint_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_HingeConstraint* self = (ik_HingeConstraint*)myself;
    struct ik_constraint* constraint = (struct ik_constraint*)self->super.super.attachment;
    ik_Vec3* axis = NULL;

    static char* kwds_str[] = {
        "axis",
        "min_angle",
        "max_angle",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!" FMT FMT, kwds_str,
                                     &ik_Vec3Type, &axis,
                                     &constraint->data.hinge.min_angle,
                                     &constraint->data.hinge.max_angle))
    {
        return -1;
    }

    if (axis != NULL)
        ASSIGN_VEC3(self->axis, axis);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
HingeConstraint_repr(PyObject* myself)
{
    PyObject* repr;
    PyObject* min_angle;
    PyObject* max_angle;
    ik_HingeConstraint* self = (ik_HingeConstraint*)myself;
    struct ik_constraint* constraint = (struct ik_constraint*)self->super.super.attachment;

    if ((min_angle = PyFloat_FromDouble(constraint->data.hinge.min_angle)) == NULL)
        goto min_angle_failed;
    if ((max_angle = PyFloat_FromDouble(constraint->data.hinge.max_angle)) == NULL)
        goto max_angle_failed;

    repr = PyUnicode_FromFormat("ik.HingeConstraint(axis=%R, min_angle=%R, max_angle=%R)", self->axis, min_angle, max_angle);
    Py_DECREF(min_angle);
    Py_DECREF(max_angle);
    return repr;

    max_angle_failed : Py_DECREF(min_angle);
    min_angle_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
HingeConstraint_str(PyObject* myself)
{
    return HingeConstraint_repr(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
HingeConstraint_getaxis(PyObject* myself, void* closure)
{
    ik_HingeConstraint* self = (ik_HingeConstraint*)myself;
    (void)closure;
    return Py_INCREF(self->axis), (PyObject*)self->axis;
}
static int
HingeConstraint_setaxis(PyObject* myself, PyObject* value, void* closure)
{
    ik_HingeConstraint* self = (ik_HingeConstraint*)myself;
    (void)closure;
    if (!ik_Vec3_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type");
        return -1;
    }

    ASSIGN_VEC3(self->axis, (ik_Vec3*)value);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
HingeConstraint_getmin_angle(PyObject* myself, void* closure)
{
    ik_HingeConstraint* self = (ik_HingeConstraint*)myself;
    struct ik_constraint* constraint = (struct ik_constraint*)self->super.super.attachment;
    (void)closure;
    return PyFloat_FromDouble(constraint->data.hinge.min_angle);
}
static int
HingeConstraint_setmin_angle(PyObject* myself, PyObject* value, void* closure)
{
    ik_HingeConstraint* self = (ik_HingeConstraint*)myself;
    struct ik_constraint* constraint = (struct ik_constraint*)self->super.super.attachment;
    (void)closure;

    if (PyFloat_Check(value))
    {
        constraint->data.hinge.min_angle = PyFloat_AS_DOUBLE(value);
        return 0;
    }
    if (PyLong_Check(value))
    {
        double d = PyLong_AsDouble(value);
        if (d == -1.0 && PyErr_Occurred())
            return -1;
        constraint->data.hinge.min_angle = d;
        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a value");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
HingeConstraint_getmax_angle(PyObject* myself, void* closure)
{
    ik_HingeConstraint* self = (ik_HingeConstraint*)myself;
    struct ik_constraint* constraint = (struct ik_constraint*)self->super.super.attachment;
    (void)closure;
    return PyFloat_FromDouble(constraint->data.hinge.max_angle);
}
static int
HingeConstraint_setmax_angle(PyObject* myself, PyObject* value, void* closure)
{
    ik_HingeConstraint* self = (ik_HingeConstraint*)myself;
    struct ik_constraint* constraint = (struct ik_constraint*)self->super.super.attachment;
    (void)closure;

    if (PyFloat_Check(value))
    {
        constraint->data.hinge.max_angle = PyFloat_AS_DOUBLE(value);
        return 0;
    }
    if (PyLong_Check(value))
    {
        double d = PyLong_AsDouble(value);
        if (d == -1.0 && PyErr_Occurred())
            return -1;
        constraint->data.hinge.max_angle = d;
        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a value");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef HingeConstraint_getsetters[] = {
    {"axis",      HingeConstraint_getaxis,      HingeConstraint_setaxis,      IK_HINGECONSTRAINT_AXIS_DOC},
    {"min_angle", HingeConstraint_getmin_angle, HingeConstraint_setmin_angle, IK_HINGECONSTRAINT_MIN_ANGLE_DOC},
    {"max_angle", HingeConstraint_getmax_angle, HingeConstraint_setmax_angle, IK_HINGECONSTRAINT_MAX_ANGLE_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject ik_HingeConstraintType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.HingeConstraint",
    .tp_basicsize = sizeof(ik_HingeConstraint),
    .tp_dealloc = HingeConstraint_dealloc,
    .tp_base = &ik_ConstraintType,
    .tp_repr = HingeConstraint_repr,
    .tp_str = HingeConstraint_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_HINGECONSTRAINT_DOC,
    .tp_getset = HingeConstraint_getsetters,
    .tp_new = HingeConstraint_new,
    .tp_init = HingeConstraint_init
};

/* ------------------------------------------------------------------------- */
int
init_ik_ConstraintType(void)
{
    ik_ConstraintType.tp_base = &ik_AttachmentType;
    if (PyType_Ready(&ik_ConstraintType) < 0)      return -1;
    if (PyType_Ready(&ik_StiffConstraintType) < 0) return -1;
    if (PyType_Ready(&ik_HingeConstraintType) < 0) return -1;
    return 0;
}
