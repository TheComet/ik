#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Node.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Solver_dealloc(ik_Solver* self)
{
    Py_TYPE(self)->tp_free(self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Solver_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    (void)kwds;
    ik_Solver* self;
    const char* algorithmName;

    if (!PyArg_ParseTuple(args, "s", &algorithmName))
        return NULL;

    self = (ik_Solver*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    /*self->algorithm = IKAPI.algorithm.create(algorithmName);*/
    if (self->algorithm == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create requested algorithm!");
        goto create_algorithm_failed;
    }

    return (PyObject*)self;

    create_algorithm_failed : Py_DECREF(self);
    alloc_self_failed    : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Solver_rebuild_data(ik_Solver* self, PyObject* arg)
{
    Py_RETURN_FALSE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Solver_calculate_segment_lengths(ik_Solver* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Solver_solve(ik_Solver* self, PyObject* arg)
{
    Py_RETURN_FALSE;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Solver_methods[] = {
    {"rebuild_data",              (PyCFunction)Solver_rebuild_data,              METH_NOARGS, "Rebuilds internal structures in the algorithm"},
    {"calculate_segment_lengths", (PyCFunction)Solver_calculate_segment_lengths, METH_NOARGS, "Updates calculated segment lenghts"},
    {"solve",                     (PyCFunction)Solver_solve,                     METH_NOARGS, "Executes the algorithm"},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Solver_repr(ik_Solver* self)
{
    (void)self;
    PyObject *fmt, *args, *str;
    if ((args = PyTuple_New(0)) == NULL) goto tuple_failed;
    if ((fmt = PyUnicode_FromString("ik.Solver()")) == NULL) goto fmt_failed;
    if ((str = PyUnicode_Format(fmt, args)) == NULL) goto str_failed;

    Py_DECREF(fmt);
    Py_DECREF(args);
    return str;

    str_failed    : Py_DECREF(fmt);
    fmt_failed    : Py_DECREF(args);
    tuple_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_SolverType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ik.Solver",                                   /* tp_name */
    sizeof(ik_Solver),                             /* tp_basicsize */
    0,                                             /* tp_itemsize */
    (destructor)Solver_dealloc,                    /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_reserved */
    (reprfunc)Solver_repr,                         /* tp_repr */
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
    Solver_methods,                                /* tp_methods */
    0,                                             /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    0,                                             /* tp_init */
    0,                                             /* tp_alloc */
    Solver_new                                     /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
init_ik_SolverType(void)
{
    if (PyType_Ready(&ik_SolverType) < 0)
        return -1;
    return 0;
}
