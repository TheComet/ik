#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/vec3.h"
#include "structmember.h"

#if defined(IK_PRECISION_DOUBLE) || defined(IK_PRECISION_LONG_DOUBLE)
#   define MEMBER_TYPE T_DOUBLE
#elif defined(IK_PRECISION_FLOAT)
#   define FMT "f"
#   define MEMBER_TYPE T_FLOAT
#else
#   error Dont know how to wrap this precision type
#endif

/* ------------------------------------------------------------------------- */
static void
Vec3_dealloc(PyObject* myself)
{
    Py_TYPE(myself)->tp_free(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Vec3* self;
    (void)args; (void)kwds;

    self = (ik_Vec3*)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    ik_vec3_set_zero(self->vec.f);

    return (PyObject*)self;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_set(PyObject* self, PyObject* args);
static int
Vec3_init(PyObject* self, PyObject* args, PyObject* kwds)
{
    (void)kwds;

    assert(PySequence_Check(args));
    if (PySequence_Fast_GET_SIZE(args) > 0)
    {
        PyObject* result = Vec3_set(self, args);
        if (result == NULL)
            return -1;
        Py_DECREF(result);
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_set(PyObject* myself, PyObject* args)
{
    ik_Vec3* self = (ik_Vec3*)myself;
    assert(PySequence_Check(args));

    if (PySequence_Fast_GET_SIZE(args) == 1)
    {
        PyObject* arg = PySequence_Fast_GET_ITEM(args, 0);
        if (ik_Vec3_CheckExact(arg))
        {
            ik_Vec3* other = (ik_Vec3*)arg;
            ik_vec3_copy(self->vec.f, other->vec.f);
            Py_RETURN_NONE;
        }
        else if (PySequence_Check(arg))
        {
            double x, y, z;
            if (!PyArg_ParseTuple(arg, "ddd", &x, &y, &z))
                return NULL;
            ik_vec3_set(self->vec.f, x, y, z);
            Py_RETURN_NONE;
        }

        PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type or a tuple/list with 3 values");
        return NULL;
    }
    else if (PySequence_Fast_GET_SIZE(args) == 3)
    {
        double x, y, z;
        if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
            return NULL;
        ik_vec3_set(self->vec.f, x, y, z);
        Py_RETURN_NONE;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type or a tuple/list with 3 values");
    return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_set_zero(PyObject* myself, PyObject* args)
{
    ik_Vec3* self = (ik_Vec3*)myself;
    (void)args;

    ik_vec3_set_zero(self->vec.f);

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_add(PyObject* myself, PyObject* arg)
{
    ik_Vec3* self = (ik_Vec3*)myself;

    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_add_vec3(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PyFloat_Check(arg))
        ik_vec3_add_scalar(self->vec.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_vec3_add_scalar(self->vec.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3 other;
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
Vec3_sub(PyObject* myself, PyObject* arg)
{
    ik_Vec3* self = (ik_Vec3*)myself;

    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_sub_vec3(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PyFloat_Check(arg))
        ik_vec3_sub_scalar(self->vec.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_vec3_sub_scalar(self->vec.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3 other;
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
Vec3_mul(PyObject* myself, PyObject* arg)
{
    ik_Vec3* self = (ik_Vec3*)myself;

    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_mul_vec3(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PyFloat_Check(arg))
        ik_vec3_mul_scalar(self->vec.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_vec3_mul_scalar(self->vec.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3 other;
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
Vec3_div(PyObject* myself, PyObject* arg)
{
    ik_Vec3* self = (ik_Vec3*)myself;

    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_div_vec3(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PyFloat_Check(arg))
        ik_vec3_div_scalar(self->vec.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_vec3_div_scalar(self->vec.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3 other;
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
Vec3_length_squared(PyObject* myself, PyObject* args)
{
    ik_Vec3* self = (ik_Vec3*)myself;
    (void)args;
    return PyFloat_FromDouble(ik_vec3_length_squared(self->vec.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_length(PyObject* myself, PyObject* args)
{
    ik_Vec3* self = (ik_Vec3*)myself;
    (void)args;
    return PyFloat_FromDouble(ik_vec3_length(self->vec.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_normalize(PyObject* myself, PyObject* args)
{
    ik_Vec3* self = (ik_Vec3*)myself;
    (void)args;
    ik_vec3_normalize(self->vec.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_dot(PyObject* myself, PyObject* arg)
{
    ik_Vec3* self = (ik_Vec3*)myself;

    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        return PyFloat_FromDouble(ik_vec3_dot(self->vec.f, ((ik_Vec3*)arg)->vec.f));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3 other;
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
Vec3_cross(PyObject* myself, PyObject* arg)
{
    ik_Vec3* self = (ik_Vec3*)myself;

    if (PyObject_TypeCheck(arg, &ik_Vec3Type))
        ik_vec3_cross(self->vec.f, ((ik_Vec3*)arg)->vec.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 3)
    {
        union ik_vec3 other;
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
Vec3_rotate(PyObject* myself, PyObject* arg)
{
    ik_Vec3* self = (ik_Vec3*)myself;

    if (PyObject_TypeCheck(arg, &ik_QuatType))
        ik_vec3_rotate_quat(self->vec.f, ((ik_Quat*)arg)->quat.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        union ik_quat other;
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
Vec3_repr(PyObject* myself)
{
    PyObject *fmt, *args, *str, *x, *y, *z;
    ik_Vec3* self = (ik_Vec3*)myself;

    if ((args = PyTuple_New(3)) == NULL) goto tuple_failed;
    if ((x = PyFloat_FromDouble(self->vec.v.x)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 0, x);
    if ((y = PyFloat_FromDouble(self->vec.v.y)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 1, y);
    if ((z = PyFloat_FromDouble(self->vec.v.z)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 2, z);
    if ((fmt = PyUnicode_FromString("ik.Vec3(%f, %f, %f)")) == NULL) goto fmt_failed;
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
    {"set_zero",       Vec3_set_zero,       METH_NOARGS,  "Sets all components to 0.0"},
    {"set",            Vec3_set,            METH_VARARGS, "Copies components from another vector or tuple"},
    {"add",            Vec3_add,            METH_O,       "Adds another vector or scalar to this vector"},
    {"sub",            Vec3_sub,            METH_O,       "Subtracts another vector or scalar from this vector"},
    {"mul",            Vec3_mul,            METH_O,       "Multiplies another vector or scalar to this vector"},
    {"div",            Vec3_div,            METH_O,       "Divides another vector or scalar to this vector"},
    {"length_squared", Vec3_length_squared, METH_NOARGS,  "Returns the squared length of the vector"},
    {"length",         Vec3_length,         METH_NOARGS,  "Returns the length of the vector"},
    {"normalize",      Vec3_normalize,      METH_NOARGS,  "Normalizes the vector"},
    {"dot",            Vec3_dot,            METH_O,       "Calculate dot product of two vectors"},
    {"cross",          Vec3_cross,          METH_O,       "Calculate cross product of two vectors"},
    {"rotate",         Vec3_rotate,         METH_O,       "Rotate a vector by a quaternion"},
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
PyDoc_STRVAR(VEC3_DOC, "");
PyTypeObject ik_Vec3Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Vec3",
    .tp_basicsize = sizeof(ik_Vec3),
    .tp_dealloc = Vec3_dealloc,
    .tp_repr = Vec3_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = VEC3_DOC,
    .tp_methods = Vec3_methods,
    .tp_members = Vec3_members,
    .tp_init = Vec3_init,
    .tp_new = Vec3_new
};

/* ------------------------------------------------------------------------- */
int
init_ik_Vec3Type(void)
{
    if (PyType_Ready(&ik_Vec3Type) < 0)
        return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
int
vec3_python_to_ik(PyObject* vpy, ikreal vik[3])
{
    if (ik_Vec3_CheckExact(vpy))
    {
        ik_Vec3* v = (ik_Vec3*)vpy;
        ik_vec3_copy(vik, v->vec.f);
    }
    else if(PySequence_Check(vpy))
    {
        if (PySequence_Fast_GET_SIZE(vpy) != 3)
        {
            PyErr_Format(PyExc_TypeError, "Expected an array of 3 values, but got %d", PySequence_Fast_GET_SIZE(vpy));
            return -1;
        }

        PyObject* x = PySequence_Fast_GET_ITEM(vpy, 0);
        PyObject* y = PySequence_Fast_GET_ITEM(vpy, 1);
        PyObject* z = PySequence_Fast_GET_ITEM(vpy, 2);
        if (!PyFloat_Check(x))
        {
            PyErr_SetString(PyExc_TypeError, "x component is not a float");
            return -1;
        }
        if (!PyFloat_Check(y))
        {
            PyErr_SetString(PyExc_TypeError, "y component is not a float");
            return -1;
        }
        if (!PyFloat_Check(z))
        {
            PyErr_SetString(PyExc_TypeError, "z component is not a float");
            return -1;
        }

        vik[0] = PyFloat_AsDouble(x);
        vik[1] = PyFloat_AsDouble(y);
        vik[2] = PyFloat_AsDouble(z);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either a ik.Vec3 type or an array of 3 floats");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
ik_Vec3*
vec3_ik_to_python(ikreal v[3])
{
    ik_Vec3* vpy = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL);
    if (vpy == NULL)
        return NULL;
    ik_vec3_copy(vpy->vec.f, v);
    return vpy;
}
