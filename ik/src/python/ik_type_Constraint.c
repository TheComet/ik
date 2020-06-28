#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/constraint.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Constraint_dealloc(PyObject* myself)
{
    ik_Constraint* self = (ik_Constraint*)myself;
    ik_ConstraintType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Constraint_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    (void)kwds;
    ik_Constraint* self;
    ik_Solver* pySolver;
    PyObject* constraint_name;

    /* We require the algorithm to know which derived constraint object to create,
     * and we need a string identifying the type of the constraint */
    if (!PyArg_ParseTuple(args, "O!U", &ik_SolverType, &pySolver, &constraint_name))
        return NULL;

    /* Allocate constraint */
    self = (ik_Constraint*)ik_ConstraintType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed          : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Constraint_methods[] = {
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject ik_ConstraintType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ik.Constraint",                               /* tp_name */
    sizeof(ik_Constraint),                         /* tp_basicsize */
    0,                                             /* tp_itemsize */
    (destructor)Constraint_dealloc,                /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_reserved */
    0,                         /* tp_repr */
    0,                                             /* tp_as_number */
    0,                                             /* tp_as_sequence */
    0,                                             /* tp_as_mapping */
    0,                                             /* tp_hash  */
    0,                                             /* tp_call */
    0,                                             /* tp_str */
    0,                                             /* tp_getattro */
    0,                                             /* tp_setattro */
    0,                                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                            /* tp_flags */
    "",                                            /* tp_doc */
    0,                                             /* tp_traverse */
    0,                                             /* tp_clear */
    0,                                             /* tp_richcompare */
    0,                                             /* tp_weaklistoffset */
    0,                                             /* tp_iter */
    0,                                             /* tp_iternext */
    Constraint_methods,                            /* tp_methods */
    0,                                             /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    0,                                             /* tp_init */
    0,                                             /* tp_alloc */
    Constraint_new                                 /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
init_ik_ConstraintType(void)
{
    if (PyType_Ready(&ik_ConstraintType) < 0)
        return -1;
    return 0;
}
