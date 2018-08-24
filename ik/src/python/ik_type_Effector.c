#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/ik.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Effector_dealloc(ik_Effector* self)
{
    if (self->effector)
        IKAPI.effector.destroy(self->effector);
    Py_DECREF(self->target_position);
    Py_DECREF(self->target_rotation);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    (void)kwds;
    ik_Effector* self;
    ik_Solver* pySolver;

    /* We require the solver to know which derived effector object to create */
    if (!PyArg_ParseTuple(args, "O!", &ik_SolverType, &pySolver))
        return NULL;

    /* Allocate effector */
    self = (ik_Effector*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    /* Allocate internal effector */
    self->effector = IKAPI.effector.create();
    if (self->effector == NULL)
        goto create_effector_failed;

    self->target_position = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL);
    if (self->target_position == NULL)
        goto alloc_target_position_failed;

    self->target_rotation = (ik_Quat*)PyObject_CallObject((PyObject*)&ik_QuatType, NULL);
    if (self->target_rotation == NULL)
        goto alloc_target_rotation_failed;

    return (PyObject*)self;

    alloc_target_rotation_failed : Py_DECREF(self->target_position);
    alloc_target_position_failed : IKAPI.effector.destroy(self->effector);
    create_effector_failed       : Py_DECREF(self);
    alloc_self_failed            : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Effector_init(ik_Effector* self, PyObject* args, PyObject* kwds)
{
    if (self->effector == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Effector was destroyed internally");
        return -1;
    }

    (void)kwds, (void)args, (void)self;
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_attach(ik_Effector* self, PyObject* pyNode)
{
    struct ik_node_t* node;

    if (self->effector == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Effector was destroyed internally");
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

    if (IKAPI.effector.attach(self->effector, node) != IK_OK)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to attach effector. Does the node already have a effector?");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_detach(ik_Effector* self, PyObject* args)
{
    (void)args;

    if (self->effector == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Effector was destroyed internally");
        return NULL;
    }

    IKAPI.effector.detach(self->effector);

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Effector_methods[] = {
    {"attach",   (PyCFunction)Effector_attach,   METH_O,      "Attaches the effector to a node in a tree"},
    {"detach",   (PyCFunction)Effector_detach,   METH_NOARGS, "Detaches the effector from a node in a tree"},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static int
Effector_settarget_position(ik_Effector* self, PyObject* value, void* closure)
{
    (void)closure;
    PyObject* target;
    PyObject* args;
    PyObject* tmp;

    args = PyTuple_New(1);
    if (args == NULL)
        goto create_args_tuple_failed;
    PyTuple_SET_ITEM(args, 0, value);

    target = PyObject_CallObject((PyObject*)&ik_Vec3Type, args);
    if (target == NULL)
        goto create_vec3_failed;

    tmp = (PyObject*)self->target_position;
    Py_INCREF(target);
    self->target_position = (ik_Vec3*)target;
    Py_DECREF(tmp);

    Py_DECREF(args);
    return 0;

    create_vec3_failed       : Py_DECREF(args);
    create_args_tuple_failed : return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_gettarget_position(ik_Effector* self, void* closure)
{
    (void)closure;
    Py_INCREF(self->target_position);
    return (PyObject*)self->target_position;
}

/* ------------------------------------------------------------------------- */
static int
Effector_settarget_rotation(ik_Effector* self, PyObject* value, void* closure)
{
    (void)closure;
    PyObject* tmp;

    if (!PyObject_TypeCheck(value, &ik_QuatType))
    {
        PyErr_SetString(PyExc_TypeError, "Expected ik.Quat() type for target rotation");
        return -1;
    }

    tmp = (PyObject*)self->target_position;
    Py_INCREF(value);
    self->target_rotation = (ik_Quat*)value;
    Py_DECREF(tmp);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_gettarget_rotation(ik_Effector* self, void* closure)
{
    (void)closure;
    Py_INCREF(self->target_rotation);
    return (PyObject*)self->target_rotation;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Effector_getsetters[] = {
    {"target_position", (getter)Effector_gettarget_position, (setter)Effector_settarget_position, ""},
    {"target_rotation", (getter)Effector_gettarget_rotation, (setter)Effector_settarget_rotation, ""},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject ik_EffectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ik.Effector",                               /* tp_name */
    sizeof(ik_Effector),                         /* tp_basicsize */
    0,                                             /* tp_itemsize */
    (destructor)Effector_dealloc,                /* tp_dealloc */
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
    Effector_methods,                              /* tp_methods */
    0,                                             /* tp_members */
    Effector_getsetters,                           /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    (initproc)Effector_init,                       /* tp_init */
    0,                                             /* tp_alloc */
    Effector_new                                   /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
init_ik_EffectorType(void)
{
    if (PyType_Ready(&ik_EffectorType) < 0)
        return -1;
    return 0;
}
