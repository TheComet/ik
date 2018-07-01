#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Node.h"
#include "ik/solver.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Constraint_dealloc(ik_Constraint* self)
{
    if (self->constraint)
        self->constraint->v->destroy(self->constraint);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Constraint_set_type(ik_Constraint* self, PyObject* args);
static PyObject*
Constraint_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    (void)kwds;
    ik_Constraint* self;
    ik_Solver* pySolver;
    PyObject* constraint_name;
    PyObject* result;

    /* We require the solver to know which derived constraint object to create,
     * and we need a string identifying the type of the constraint */
    if (!PyArg_ParseTuple(args, "O!U", &ik_SolverType, &pySolver, &constraint_name))
        return NULL;

    /* Allocate constraint */
    self = (ik_Constraint*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    self->constraint = pySolver->solver->constraint->create(IK_NONE);
    if (self->constraint == NULL)
        goto create_constraint_failed;

    result = Constraint_set_type(self, constraint_name);
    if (result == NULL)
        goto set_constraint_type_failed;
    Py_DECREF(result);

    return (PyObject*)self;

    set_constraint_type_failed : self->constraint->v->destroy(self->constraint);
    create_constraint_failed   : Py_DECREF(self);
    alloc_self_failed          : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Constraint_init(ik_Constraint* self, PyObject* args, PyObject* kwds)
{
    if (self->constraint == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Constraint was destroyed internally");
        return -1;
    }

    (void)kwds, (void)args, (void)self;
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Constraint_set_type(ik_Constraint* self, PyObject* arg)
{
    PyObject* ascii_name;

    if (self->constraint == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Constraint was destroyed internally");
        return NULL;
    }

    if ((ascii_name = PyUnicode_AsASCIIString(arg)) == NULL)
        goto convert_to_ascii_failed;

    /* Map string to ik_constraint_type_e enumeration and create appropriate
     * constraint */
    if (0) {}
#define X(type) \
            else if (strcmp(PyBytes_AS_STRING(ascii_name), #type) == 0) { \
                if (self->constraint->v->set_type(self->constraint, IK_##type) != IK_OK) { \
                    PyErr_SetString(PyExc_TypeError, "Failed to set constraint type. Did you use the correct method?"); \
                    goto set_constraint_type_failed; \
                } \
            }
    IK_CONSTRAINTS
#undef X
    else
    {
        PyErr_SetString(PyExc_TypeError, "Unknown constraint type. Exepected one of the following: "
#define X(type) #type ", "
        IK_CONSTRAINTS
#undef X
        );
        goto set_constraint_type_failed;
    }

    Py_DECREF(ascii_name);
    Py_RETURN_NONE;

    set_constraint_type_failed : Py_DECREF(ascii_name);
    convert_to_ascii_failed    : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Constraint_attach(ik_Constraint* self, PyObject* pyNode)
{
    struct ik_node_t* node;

    if (self->constraint == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Constraint was destroyed internally");
        return NULL;
    }

    if (!PyObject_TypeCheck(pyNode, &ik_NodeType))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a node of type ik.Node() to attach to.");
        return NULL;
    }

    node = ((ik_Node*)pyNode)->node;
    if (node == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "The node you are trying to attach to was destroyed internally");
        return NULL;
    }

    if (self->constraint->v->attach(self->constraint, node) != IK_OK)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to attach constraint. Does the node already have a constraint?");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Constraint_detach(ik_Constraint* self, PyObject* args)
{
    (void)args;

    if (self->constraint == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Constraint was destroyed internally");
        return NULL;
    }

    self->constraint->v->detach(self->constraint);

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Constraint_methods[] = {
    {"set_type", (PyCFunction)Constraint_set_type, METH_O,      "Sets the type of the constraint"},
    {"attach",   (PyCFunction)Constraint_attach,   METH_O,      "Attaches the constraint to a node in a tree"},
    {"detach",   (PyCFunction)Constraint_detach,   METH_NOARGS, "Detaches the constraint from a node in a tree"},
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
    (initproc)Constraint_init,                     /* tp_init */
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
