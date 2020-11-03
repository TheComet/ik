#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_helpers.h"
#include "ik/python/ik_docstrings.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"
#include "ik/node.h"

/* ------------------------------------------------------------------------- */
static void
Node_dealloc(PyObject* myself)
{
    ik_Node* self = (ik_Node*)myself;

    Py_DECREF(self->position);
    Py_DECREF(self->rotation);

    ik_NodeType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Node* self;
    ik_Vec3* position;
    ik_Quat* rotation;
    struct ik_node* node;
    PyObject* node_capsule;
    PyObject* base_args;
    Py_ssize_t i;

    /* Allocate internal node */
    if ((node = ik_node_create()) == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate internal node");
        goto alloc_node_failed;
    }
    IK_INCREF(node);

    /* Allocate members */
    position = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL);
    if (position == NULL)
        goto alloc_position_failed;
    rotation = (ik_Quat*)PyObject_CallObject((PyObject*)&ik_QuatType, NULL);
    if (rotation == NULL)
        goto alloc_rotation_failed;

    /* Prepend internal node instance to the arg list for instantiating the base
     * class */
    node_capsule = PyCapsule_New(node, NULL, NULL);
    if (node_capsule == NULL)
        goto alloc_base_args_failed;
    base_args = PyTuple_New(PyTuple_GET_SIZE(args) + 1);
    if (base_args == NULL)
    {
        Py_DECREF(node_capsule);
        goto alloc_base_args_failed;
    }
    PyTuple_SET_ITEM(base_args, 0, node_capsule);  /* steals ref */
    for (i = 0; i != PyTuple_GET_SIZE(args); ++i)
    {
        PyObject* item = PyTuple_GET_ITEM(args, i);
        Py_INCREF(item);
        PyTuple_SET_ITEM(base_args, i+1, item);
    }

    /* Finally, allocate self */
    self = (ik_Node*)ik_NodeType.tp_base->tp_new(type, base_args, kwds);
    Py_DECREF(base_args);
    if (self == NULL)
        goto alloc_self_failed;

    /*
     * Base class is holding a ref to the internal node in self->super.tree_object
     * so we don't have to. The reason we incref it at all is so decref works
     * in case of a failure. Safe to decref it again.
     */
    IK_DECREF(node);

    /* Assign members */
    self->position = position;
    self->rotation = rotation;

    REF_VEC3_DATA(self->position, &node->position);
    REF_QUAT_DATA(self->rotation, &node->rotation);

    return (PyObject*)self;

    alloc_self_failed      :
    alloc_base_args_failed : Py_DECREF(rotation);
    alloc_rotation_failed  : Py_DECREF(position);
    alloc_position_failed  : IK_DECREF(node);
    alloc_node_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Node_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Node* self = (ik_Node*)myself;
    ik_Vec3* position = NULL;
    ik_Quat* rotation = NULL;

    static char* kwds_str[] = {
        "position",
        "rotation",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!O!", kwds_str,
        &ik_Vec3Type, &position,
        &ik_QuatType, &rotation))
    {
        return -1;
    }

    if (position)
        ASSIGN_VEC3(self->position, position);
    if (rotation)
        ASSIGN_QUAT(self->rotation, rotation);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_getposition(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return Py_INCREF(self->position), (PyObject*)self->position;
}
static int
Node_setposition(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    if (!ik_Vec3_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type for position");
        return -1;
    }

    ASSIGN_VEC3(self->position, (ik_Vec3*)value);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_getrotation(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return Py_INCREF(self->rotation), (PyObject*)self->rotation;
}
static int
Node_setrotation(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    if (!ik_Quat_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Quat() type for rotation");
        return -1;
    }

    ASSIGN_QUAT(self->rotation, (ik_Quat*)value);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Node_getsetters[] = {
    {"position",      Node_getposition, Node_setposition, IK_BONE_POSITION_DOC},
    {"rotation",      Node_getrotation, Node_setrotation, IK_BONE_ROTATION_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static int
Node_repr_prepend_arglist(PyObject* myself, PyObject* args)
{
    ik_Node* self = (ik_Node*)myself;
    struct ik_node* node = (struct ik_node*)self->super.tree_object;

    /* position */
    {
        int append_result;
        PyObject* position;
        PyObject* arg;

        position = (PyObject*)vec3_ik_to_python(node->position.f);
        if (position == NULL)
            return -1;

        arg = PyUnicode_FromFormat("position=%R", position);
        Py_DECREF(position);
        if (arg == NULL)
            return -1;

        append_result = PyList_Insert(args, 0, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            return -1;
    }

    /* rotation */
    {
        int append_result;
        PyObject* rotation;
        PyObject* arg;

        rotation = (PyObject*)quat_ik_to_python(node->rotation.f);
        if (rotation == NULL)
            return -1;

        arg = PyUnicode_FromFormat("rotation=%R", rotation);
        Py_DECREF(rotation);
        if (arg == NULL)
            return -1;

        append_result = PyList_Insert(args, 1, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            return -1;
    }

    return 0;
}
static PyObject*
Node_repr_build_arglist_string(PyObject* myself)
{
    PyObject* separator;
    PyObject* arglist;
    PyObject* string;

    separator = PyUnicode_FromString(", ");
    if (separator == NULL)
        return NULL;

    arglist = TreeObject_repr_build_arglist_list(myself);
    if (arglist == NULL || Node_repr_prepend_arglist(myself, arglist) != 0)
    {
        Py_DECREF(separator);
        return NULL;
    }

    string = PyUnicode_Join(separator, arglist);
    Py_DECREF(separator);
    Py_DECREF(arglist);
    return string;
}
static PyObject*
Node_repr(PyObject* myself)
{
    PyObject* repr;
    PyObject* argstring = Node_repr_build_arglist_string(myself);
    if (argstring == NULL)
        return NULL;

    repr = PyUnicode_FromFormat("ik.Node(%U)", argstring);
    Py_DECREF(argstring);
    return repr;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_str(PyObject* myself)
{
    return Node_repr(myself);
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_NodeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Node",
    .tp_basicsize = sizeof(ik_Node),
    .tp_dealloc = Node_dealloc,
    .tp_repr = Node_repr,
    .tp_str = Node_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_BONE_DOC,
    .tp_getset = Node_getsetters,
    .tp_new = Node_new,
    .tp_init = Node_init
};

/* ------------------------------------------------------------------------- */
int
init_ik_NodeType(void)
{
    ik_NodeType.tp_base = &ik_TreeObjectType;
    if (PyType_Ready(&ik_NodeType) < 0)
        return -1;
    return 0;
}

