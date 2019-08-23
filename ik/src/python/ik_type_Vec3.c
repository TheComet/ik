#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/ik.h"
#include "structmember.h"

#if defined(IK_PRECISION_DOUBLE) || defined(IK_PRECISION_LONG_DOUBLE)
#   define FMT "d"
#   define MEMBER_TYPE T_DOUBLE
#elif defined(IK_PRECISION_FLOAT)
#   define FMT "f"
#   define MEMBER_TYPE T_FLOAT
#else
#   error Dont know how to wrap this precision type
#endif

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_copy(ik_Vec3* self, PyObject* arg);
static int
Vec3_init(ik_Vec3* self, PyObject* args, PyObject* kwds)
{
    PyObject* position = NULL;
    (void)kwds;

    ik_vec3_set_zero(self->vec.f);
    if (!PyArg_ParseTuple(args, "|O", &position))
        return -1;

    if (position && Vec3_copy(self, position) == NULL)
        return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_copy(ik_Vec3* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
    {
        ik_vec3_copy(self->vec.f, ((ik_Vec3*)arg)->vec.f);
        Py_RETURN_NONE;
    }
    else if (PyArg_ParseTuple(arg, FMT FMT FMT, &self->vec.v.x, &self->vec.v.y, &self->vec.v.z))
    {
        Py_RETURN_NONE;
    }

    return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_set_zero(ik_Vec3* self, PyObject* args)
{
    (void)args;
    ik_vec3_set_zero(self->vec.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_add(ik_Vec3* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_add_vec3(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PyFloat_Check(arg))
        ik_vec3_add_scalar(self->vec.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_vec3_add_scalar(self->vec.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3_t other;
        other.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        if (PyErr_Occurred())
            return NULL;
        ik_vec3_add_vec3(self->vec.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Vec3 type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_sub(ik_Vec3* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_sub_vec3(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PyFloat_Check(arg))
        ik_vec3_sub_scalar(self->vec.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_vec3_sub_scalar(self->vec.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3_t other;
        other.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        if (PyErr_Occurred())
            return NULL;
        ik_vec3_sub_vec3(self->vec.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Vec3 type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_mul(ik_Vec3* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_mul_vec3(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PyFloat_Check(arg))
        ik_vec3_mul_scalar(self->vec.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_vec3_mul_scalar(self->vec.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3_t other;
        other.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        if (PyErr_Occurred())
            return NULL;
        ik_vec3_mul_vec3(self->vec.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Vec3 type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_div(ik_Vec3* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_div_vec3(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PyFloat_Check(arg))
        ik_vec3_div_scalar(self->vec.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_vec3_div_scalar(self->vec.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3_t other;
        other.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        if (PyErr_Occurred())
            return NULL;
        ik_vec3_div_vec3(self->vec.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Vec3 type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_length_squared(ik_Vec3* self, PyObject* args)
{
    (void)args;
    return PyFloat_FromDouble(ik_vec3_length_squared(self->vec.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_length(ik_Vec3* self, PyObject* args)
{
    (void)args;
    return PyFloat_FromDouble(ik_vec3_length(self->vec.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_normalize(ik_Vec3* self, PyObject* args)
{
    (void)args;
    ik_vec3_normalize(self->vec.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_dot(ik_Vec3* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        return PyFloat_FromDouble(ik_vec3_dot(self->vec.f, ((ik_Vec3*)arg)->vec.f));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3_t other;
        other.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        if (PyErr_Occurred())
            return NULL;
        return PyFloat_FromDouble(ik_vec3_dot(self->vec.f, other.f));
    }

    PyErr_SetString(PyExc_TypeError, "Expected either another Vec3 type or a tuple of 3 floats");
    return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_cross(ik_Vec3* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_cross(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3_t other;
        other.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        if (PyErr_Occurred())
            return NULL;
        ik_vec3_cross(self->vec.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Vec3 type or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_rotate(ik_Vec3* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_QuatType))
        ik_vec3_rotate_quat(self->vec.f, ((ik_Quat*)arg)->quat.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        union ik_quat_t other;
        other.q.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.q.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.q.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.q.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        ik_vec3_rotate_quat(self->vec.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either a IK.Quat type or a tuple of 4 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_repr(ik_Vec3* self)
{
    PyObject *fmt, *args, *str, *x, *y, *z;
    if ((args = PyTuple_New(3)) == NULL) goto tuple_failed;
    if ((x = PyFloat_FromDouble(self->vec.v.x)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 0, x);
    if ((y = PyFloat_FromDouble(self->vec.v.y)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 1, y);
    if ((z = PyFloat_FromDouble(self->vec.v.z)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 2, z);
    if ((fmt = PyUnicode_FromString("Vec3(%f, %f, %f)")) == NULL) goto fmt_failed;
    if ((str = PyUnicode_Format(fmt, args)) == NULL) goto str_failed;

    Py_DECREF(fmt);
    Py_DECREF(args);
    return str;

    str_failed    : Py_DECREF(fmt);
    fmt_failed    :
    insert_failed : Py_DECREF(args);
    tuple_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Vec3_methods[] = {
    {"set_zero",       (PyCFunction)Vec3_set_zero,       METH_NOARGS, "Sets all components to 0.0"},
    {"copy",           (PyCFunction)Vec3_copy,           METH_O,      "Copies components from another vector or tuple"},
    {"add",            (PyCFunction)Vec3_add,            METH_O,      "Adds another vector or scalar to this vector"},
    {"sub",            (PyCFunction)Vec3_sub,            METH_O,      "Subtracts another vector or scalar from this vector"},
    {"mul",            (PyCFunction)Vec3_mul,            METH_O,      "Multiplies another vector or scalar to this vector"},
    {"div",            (PyCFunction)Vec3_div,            METH_O,      "Divides another vector or scalar to this vector"},
    {"length_squared", (PyCFunction)Vec3_length_squared, METH_NOARGS, "Returns the squared length of the vector"},
    {"length",         (PyCFunction)Vec3_length,         METH_NOARGS, "Returns the length of the vector"},
    {"normalize",      (PyCFunction)Vec3_normalize,      METH_NOARGS, "Normalizes the vector"},
    {"dot",            (PyCFunction)Vec3_dot,            METH_O,      "Calculate dot product of two vectors"},
    {"cross",          (PyCFunction)Vec3_cross,          METH_O,      "Calculate cross product of two vectors"},
    {"rotate",         (PyCFunction)Vec3_rotate,         METH_O,      "Rotate a vector by a quaternion"},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyMemberDef Vec3_members[] = {
    {"x", MEMBER_TYPE, offsetof(ik_Vec3, vec.v.x), 0, "X component"},
    {"y", MEMBER_TYPE, offsetof(ik_Vec3, vec.v.y), 0, "Y component"},
    {"z", MEMBER_TYPE, offsetof(ik_Vec3, vec.v.z), 0, "Z component"},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject ik_Vec3Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ik.Vec3",                                     /* tp_name */
    sizeof(ik_Vec3),                               /* tp_basicsize */
    0,                                             /* tp_itemsize */
    0,                                             /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_reserved */
    (reprfunc)Vec3_repr,                           /* tp_repr */
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
    Vec3_methods,                                  /* tp_methods */
    Vec3_members,                                  /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    (initproc)Vec3_init,                           /* tp_init */
    0,                                             /* tp_alloc */
    0                                              /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
init_ik_Vec3Type(void)
{
    ik_Vec3Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&ik_Vec3Type) < 0)
        return -1;
    return 0;
}
