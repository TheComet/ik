#include "ik/python/ik_type_Bone.h"

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
Bone_gettransform(PyObject* myself, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
    (void)closure;
    Py_RETURN_NONE;
}
static int
Bone_settransform(PyObject* myself, PyObject* value, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
    (void)closure;
    if (!ik_Quat_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Quat() type for transform");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Bone_getglobal_position(PyObject* myself, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
    (void)closure;
    return Py_INCREF(self->position), (PyObject*)self->position;
}
static int
Bone_setglobal_position(PyObject* myself, PyObject* value, void* closure)
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
Bone_getglobal_rotation(PyObject* myself, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
    (void)closure;
    return Py_INCREF(self->rotation), (PyObject*)self->rotation;
}
static int
Bone_setglobal_rotation(PyObject* myself, PyObject* value, void* closure)
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
Bone_getglobal_transform(PyObject* myself, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
    (void)closure;
    Py_RETURN_NONE;
}
static int
Bone_setglobal_transform(PyObject* myself, PyObject* value, void* closure)
{
    ik_Bone* self = (ik_Bone*)myself;
    (void)closure;
    if (!ik_Quat_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Quat() type for transform");
        return -1;
    }

    return 0;
}

