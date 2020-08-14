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

    Py_DECREF(self->root);
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
    self->root = root;
    Py_INCREF(root);

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
Solver_repr(PyObject* myself)
{
    ik_Solver* self = (ik_Solver*)myself;

    return PyUnicode_FromFormat("ik.Solver(root=%R)", self->root);
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_SolverType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Solver",
    .tp_basicsize = sizeof(ik_Solver),
    .tp_dealloc = Solver_dealloc,
    .tp_repr = Solver_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = IK_SOLVER_DOC,
    .tp_methods = Solver_methods,
    .tp_new = Solver_new
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
