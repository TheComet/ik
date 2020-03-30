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
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    (void)kwds;
    ik_Effector* self;
    ik_Solver* pySolver;

    /* We require the algorithm to know which derived effector object to create */
    if (!PyArg_ParseTuple(args, "O!", &ik_SolverType, &pySolver))
        return NULL;

    /* Allocate effector */
    self = (ik_Effector*)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    return (PyObject*)self;
}

/* ------------------------------------------------------------------------- */
static int
Effector_settarget_position(ik_Effector* self, PyObject* value, void* closure)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_gettarget_position(ik_Effector* self, void* closure)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static int
Effector_settarget_rotation(ik_Effector* self, PyObject* value, void* closure)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_gettarget_rotation(ik_Effector* self, void* closure)
{
    Py_RETURN_NONE;
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
    0,                              /* tp_methods */
    0,                                             /* tp_members */
    Effector_getsetters,                           /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    0,                       /* tp_init */
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
