#include "ik/python/ik_type_Bone.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_helpers.h"
#include "ik/python/ik_docstrings.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"
#include "ik/bone.h"

/* ------------------------------------------------------------------------- */
static void
Bone_dealloc(PyObject* myself)
{
    ik_Bone* self = (ik_Bone*)myself;

    Py_DECREF(self->position);
    Py_DECREF(self->rotation);

    ik_BoneType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Bone_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Bone* self;
    ik_Vec3* position;
    ik_Quat* rotation;
    struct ik_bone* bone;
    PyObject* bone_capsule;
    PyObject* base_args;
    Py_ssize_t i;

    /* Allocate internal bone */
    if ((bone = ik_bone_create(0)) == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate internal bone");
        goto alloc_bone_failed;
    }
    IK_INCREF(bone);

    /* Allocate members */
    position = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL);
    if (position == NULL)
        goto alloc_position_failed;
    rotation = (ik_Quat*)PyObject_CallObject((PyObject*)&ik_QuatType, NULL);
    if (rotation == NULL)
        goto alloc_rotation_failed;

    /* Prepend internal bone instance to the arg list for instantiating the base
     * class */
    bone_capsule = PyCapsule_New(bone, NULL, NULL);
    if (bone_capsule == NULL)
        goto alloc_base_args_failed;
    base_args = PyTuple_New(PyTuple_GET_SIZE(args) + 1);
    if (base_args == NULL)
    {
        Py_DECREF(bone_capsule);
        goto alloc_base_args_failed;
    }
    PyTuple_SET_ITEM(base_args, 0, bone_capsule);  /* steals ref */
    for (i = 0; i != PyTuple_GET_SIZE(args); ++i)
    {
        PyObject* item = PyTuple_GET_ITEM(args, i);
        Py_INCREF(item);
        PyTuple_SET_ITEM(base_args, i+1, item);
    }

    /* Finally, allocate self */
    self = (ik_Bone*)ik_BoneType.tp_base->tp_new(type, base_args, kwds);
    Py_DECREF(base_args);
    if (self == NULL)
        goto alloc_self_failed;

    /*
     * Base class is holding a ref to the internal bone in self->super.tree_object
     * so we don't have to. The reason we incref it at all is so decref works
     * in case of a failure. Safe to decref it again.
     */
    IK_DECREF(bone);

    /* Assign members */
    self->position = position;
    self->rotation = rotation;

    REF_VEC3_DATA(self->position, &bone->position);
    REF_QUAT_DATA(self->rotation, &bone->rotation);

    return (PyObject*)self;

    alloc_self_failed      :
    alloc_base_args_failed : Py_DECREF(rotation);
    alloc_rotation_failed  : Py_DECREF(position);
    alloc_position_failed  : IK_DECREF(bone);
    alloc_bone_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Bone_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Bone* self = (ik_Bone*)myself;
    struct ik_bone* bone = (struct ik_bone*)self->super.tree_object;
    ik_Vec3* position = NULL;
    ik_Quat* rotation = NULL;

    static char* kwds_str[] = {
        "position",
        "rotation",
        "length",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!O!" FMT, kwds_str,
        &ik_Vec3Type, &position,
        &ik_QuatType, &rotation,
        &bone->length))
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
Bone_getposition(PyObject* myself, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
    (void)closure;
    return Py_INCREF(self->position), (PyObject*)self->position;
}
static int
Bone_setposition(PyObject* myself, PyObject* value, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
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
Bone_getrotation(PyObject* myself, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
    (void)closure;
    return Py_INCREF(self->rotation), (PyObject*)self->rotation;
}
static int
Bone_setrotation(PyObject* myself, PyObject* value, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
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
static PyObject*
Bone_getlength(PyObject* myself, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
    struct ik_bone* bone = (struct ik_bone*)self->super.tree_object;
    (void)closure;
    return PyFloat_FromDouble(bone->length);
}
static int
Bone_setlength(PyObject* myself, PyObject* value, void* closure)
{
    PyObject* as_float;
    ik_Bone* self = (ik_Bone*)myself;
    struct ik_bone* bone = (struct ik_bone*)self->super.tree_object;
    (void)closure;

    if ((as_float = PyNumber_Float(value)) == NULL)
        return -1;

    bone->length = PyFloat_AS_DOUBLE(as_float);
    Py_DECREF(as_float);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Bone_getsetters[] = {
    {"position",      Bone_getposition, Bone_setposition, IK_BONE_POSITION_DOC},
    {"rotation",      Bone_getrotation, Bone_setrotation, IK_BONE_ROTATION_DOC},
    {"length",        Bone_getlength,   Bone_setlength,   IK_BONE_LENGTH_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Bone_head_position(PyObject* myself, PyObject* args)
{
    ik_Bone* self;
    ik_Vec3* pos;
    (void)args;

    pos = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL);
    if (pos == NULL)
        return NULL;

    self = (ik_Bone*)myself;
    ik_bone_head_position((struct ik_bone*)self->super.tree_object, pos->vec.f);

    return (PyObject*)pos;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Bone_methods[] = {
    {"head_position", Bone_head_position, METH_NOARGS, IK_BONE_HEAD_POSITION_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static int
Bone_repr_prepend_arglist(PyObject* myself, PyObject* args)
{
    ik_Bone* self = (ik_Bone*)myself;
    struct ik_bone* bone = (struct ik_bone*)self->super.tree_object;

    /* position */
    {
        int append_result;
        PyObject* position;
        PyObject* arg;

        position = (PyObject*)vec3_ik_to_python(bone->position.f);
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

        rotation = (PyObject*)quat_ik_to_python(bone->rotation.f);
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

    /* length */
    {
        int append_result;
        PyObject* length;
        PyObject* arg;

        length = PyFloat_FromDouble(bone->length);
        if (length == NULL)
            return -1;

        arg = PyUnicode_FromFormat("length=%R", length);
        Py_DECREF(length);
        if (arg == NULL)
            return -1;

        append_result = PyList_Insert(args, 2, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            return -1;
    }

    return 0;
}
static PyObject*
Bone_repr_build_arglist_string(PyObject* myself)
{
    PyObject* separator;
    PyObject* arglist;
    PyObject* string;

    separator = PyUnicode_FromString(", ");
    if (separator == NULL)
        return NULL;

    arglist = TreeObject_repr_build_arglist_list(myself);
    if (arglist == NULL || Bone_repr_prepend_arglist(myself, arglist) != 0)
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
Bone_repr(PyObject* myself)
{
    PyObject* repr;
    PyObject* argstring = Bone_repr_build_arglist_string(myself);
    if (argstring == NULL)
        return NULL;

    repr = PyUnicode_FromFormat("ik.Bone(%U)", argstring);
    Py_DECREF(argstring);
    return repr;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Bone_str(PyObject* myself)
{
    return Bone_repr(myself);
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_BoneType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Bone",
    .tp_basicsize = sizeof(ik_Bone),
    .tp_dealloc = Bone_dealloc,
    .tp_repr = Bone_repr,
    .tp_str = Bone_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_BONE_DOC,
    .tp_methods = Bone_methods,
    .tp_getset = Bone_getsetters,
    .tp_new = Bone_new,
    .tp_init = Bone_init
};

/* ------------------------------------------------------------------------- */
int
init_ik_BoneType(void)
{
    ik_BoneType.tp_base = &ik_TreeObjectType;
    if (PyType_Ready(&ik_BoneType) < 0)
        return -1;
    return 0;
}
