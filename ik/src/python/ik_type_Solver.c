#include "ik/solver.h"
#include "ik/python/ik_type_ModuleRef.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_docstrings.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Solver_dealloc(PyObject* myself)
{
    ik_Solver* self = (ik_Solver*)myself;
    IK_DECREF(self->solver);
    ik_SolverType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Solver_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    struct ik_solver* solver;
    ik_Solver* self;
    ik_Node* root;

    static char* kwds_names[] = {
        "root",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwds_names,
            &ik_NodeType, &root))
        return NULL;

    if ((solver = ik_solver_build(root->node)) == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to build solver(s). Check log output for more information.");
        goto build_solvers_failed;
    }
    IK_INCREF(solver);

    self = (ik_Solver*)ik_SolverType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    self->solver = solver;

    return (PyObject*)self;

    alloc_self_failed    : IK_DECREF(solver);
    build_solvers_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Solver_solve(ik_Solver* self, PyObject* arg)
{
    (void)arg;
    return PyLong_FromLong(
        ik_solver_solve(self->solver));
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Solver_methods[] = {
    {"solve",                  (PyCFunction)Solver_solve, METH_NOARGS, IK_SOLVER_SOLVE_DOC},
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
    IK_SOLVER_DOC,                                 /* tp_doc */
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
    ik_SolverType.tp_base = &ik_ModuleRefType;
    if (PyType_Ready(&ik_SolverType) < 0)
        return -1;
    return 0;
}
