#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/quat.h"
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
Quat_dealloc(PyObject* myself)
{
    Py_TYPE(myself)->tp_free(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Quat* self;
    (void)args; (void)kwds;

    self = (ik_Quat*)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    ik_quat_set_identity(self->quat.f);

    return (PyObject*)self;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_set(PyObject* myself, PyObject* arg);
static int
Quat_init(PyObject* self, PyObject* args, PyObject* kwds)
{
    (void)kwds;

    assert(PySequence_Check(args));
    if (PySequence_Fast_GET_SIZE(args) > 0)
    {
        PyObject* result = Quat_set(self, args);
        if (result == NULL)
            return -1;
        Py_DECREF(result);
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_set_identity(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;
    (void)arg;
    ik_quat_set_identity(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_set(PyObject* myself, PyObject* args)
{
    ik_Quat* self = (ik_Quat*)myself;
    assert(PySequence_Check(args));

    if (PySequence_Fast_GET_SIZE(args) == 1)
    {
        PyObject* arg = PySequence_Fast_GET_ITEM(args, 0);
        if (ik_Quat_CheckExact(arg))
        {
            ik_Quat* other = (ik_Quat*)arg;
            ik_quat_copy(self->quat.f, other->quat.f);
            Py_RETURN_NONE;
        }
        else if (PySequence_Check(arg))
        {
            double x, y, z, w;
            if (!PyArg_ParseTuple(arg, "dddd", &x, &y, &z, &w))
                return NULL;
            ik_quat_set(self->quat.f, x, y, z, w);
            Py_RETURN_NONE;
        }

        PyErr_SetString(PyExc_TypeError, "Expected a ik.Quat() type or a tuple/list with 3 values");
        return NULL;
    }
    else if (PySequence_Fast_GET_SIZE(args) == 4)
    {
        double x, y, z, w;
        if (!PyArg_ParseTuple(args, "dddd", &x, &y, &z, &w))
            return NULL;
        ik_quat_set(self->quat.f, x, y, z, w);
        Py_RETURN_NONE;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a ik.Quat() type or a tuple/list with 3 values");
    return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_add(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;

    if (PyObject_TypeCheck(arg, &ik_QuatType))
        ik_quat_add_quat(self->quat.f, ((ik_Quat*)arg)->quat.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        union ik_quat other;
        other.q.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.q.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.q.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.q.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        ik_quat_add_quat(self->quat.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Quat type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_mag(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;
    (void)arg;
    return PyFloat_FromDouble(ik_quat_mag(self->quat.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_conj(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;
    (void)arg;
    ik_quat_conj(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_invert_sign(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;
    (void)arg;
    ik_quat_negate(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_normalize(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;
    (void)arg;
    ik_quat_normalize(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_mul(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;

    if (PyObject_TypeCheck(arg, &ik_QuatType))
        ik_quat_mul_quat(self->quat.f, ((ik_Quat*)arg)->quat.f);
    else if (PyFloat_Check(arg))
        ik_quat_mul_scalar(self->quat.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_quat_mul_scalar(self->quat.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        union ik_quat other;
        other.q.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.q.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.q.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.q.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        ik_quat_mul_quat(self->quat.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Quat type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_div(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;

    if (PyFloat_Check(arg))
        ik_quat_div_scalar(self->quat.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        ik_quat_div_scalar(self->quat.f, PyLong_AS_LONG(arg));
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Quat type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_dot(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;

    if (PyObject_TypeCheck(arg, &ik_QuatType))
        ik_quat_dot(self->quat.f, ((ik_Quat*)arg)->quat.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        union ik_quat other;
        other.q.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.q.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.q.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.q.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        ik_quat_dot(self->quat.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Quat type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_ensure_positive_sign(PyObject* myself, PyObject* arg)
{
    ik_Quat* self = (ik_Quat*)myself;
    (void)arg;
    ik_quat_ensure_positive_sign(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_angle(PyObject* myself, PyObject* args)
{
    PyObject *vec1, *vec2;
    ik_Quat* self = (ik_Quat*)myself;

    if (PyTuple_GET_SIZE(args) != 2)
    {
        PyErr_SetString(PyExc_TypeError, "Wrong number of arguments, expected two vectors.");
        return NULL;
    }

    vec1 = PyTuple_GET_ITEM(args, 0);
    vec2 = PyTuple_GET_ITEM(args, 1);

    if (PyObject_TypeCheck(vec1, &ik_Vec3Type))
    {
        if (PyObject_TypeCheck(vec2, &ik_Vec3Type))
        {
            ik_quat_angle_between(self->quat.f, ((ik_Vec3*)vec1)->vec.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            union ik_vec3 other;
            other.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            ik_quat_angle_between(self->quat.f, ((ik_Vec3*)vec1)->vec.f, other.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a Quat type or a tuple with 4 floats");
            return NULL;
        }
    }
    else if (PySequence_Check(vec1) && PySequence_Fast_GET_SIZE(vec1) == 3)
    {
        union ik_vec3 other1;
        other1.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 0));
        other1.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 1));
        other1.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 2));
        if (PyErr_Occurred())
            return NULL;

        if (PyObject_TypeCheck(vec2, &ik_Vec3Type))
        {
            ik_quat_angle_between(self->quat.f, other1.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            union ik_vec3 other2;
            other2.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other2.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other2.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            ik_quat_angle_between(self->quat.f, other1.f, other2.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a Quat type or a tuple with 4 floats");
            return NULL;
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either a Quat type or a tuple with 4 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_angle_no_normalize(PyObject* myself, PyObject* args)
{
    PyObject *vec1, *vec2;
    ik_Quat* self = (ik_Quat*)myself;

    if (PyTuple_GET_SIZE(args) != 2)
    {
        PyErr_SetString(PyExc_TypeError, "Wrong number of arguments, expected two vectors.");
        return NULL;
    }

    vec1 = PyTuple_GET_ITEM(args, 0);
    vec2 = PyTuple_GET_ITEM(args, 1);

    if (PyObject_TypeCheck(vec1, &ik_Vec3Type))
    {
        if (PyObject_TypeCheck(vec2, &ik_Vec3Type))
        {
            ik_quat_angle_between_no_normalize(self->quat.f, ((ik_Vec3*)vec1)->vec.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            union ik_vec3 other;
            other.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            ik_quat_angle_between_no_normalize(self->quat.f, ((ik_Vec3*)vec1)->vec.f, other.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a Quat type or a tuple with 4 floats");
            return NULL;
        }
    }
    else if (PySequence_Check(vec1) && PySequence_Fast_GET_SIZE(vec1) == 3)
    {
        union ik_vec3 other1;
        other1.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 0));
        other1.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 1));
        other1.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 2));
        if (PyErr_Occurred())
            return NULL;

        if (PyObject_TypeCheck(vec2, &ik_Vec3Type))
        {
            ik_quat_angle_between_no_normalize(self->quat.f, other1.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            union ik_vec3 other2;
            other2.v.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other2.v.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other2.v.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            ik_quat_angle_between_no_normalize(self->quat.f, other1.f, other2.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a Quat type or a tuple with 4 floats");
            return NULL;
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either a Quat type or a tuple with 4 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_repr(PyObject* myself)
{
    PyObject *fmt, *args, *str, *w, *x, *y, *z;
    ik_Quat* self = (ik_Quat*)myself;

    if ((args = PyTuple_New(4)) == NULL) goto tuple_failed;
    if ((w = PyFloat_FromDouble(self->quat.q.w)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 0, w);
    if ((x = PyFloat_FromDouble(self->quat.q.x)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 1, x);
    if ((y = PyFloat_FromDouble(self->quat.q.y)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 2, y);
    if ((z = PyFloat_FromDouble(self->quat.q.z)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 3, z);
    if ((fmt = PyUnicode_FromString("ik.Quat(%f, %f, %f, %f)")) == NULL) goto fmt_failed;
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
static PyMethodDef Quat_methods[] = {
    {"set_identity",        Quat_set_identity,        METH_NOARGS,  "Sets all components to 0.0"},
    {"set",                 Quat_set,                 METH_VARARGS, "Copies components from another vector or tuple"},
    {"add",                 Quat_add,                 METH_O,       "Adds another vector or scalar to this vector"},
    {"mag",                 Quat_mag,                 METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"conj",                Quat_conj,                METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"invert_sign",         Quat_invert_sign,         METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"normalize",           Quat_normalize,           METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"mul",                 Quat_mul,                 METH_O,       "Adds another vector or scalar to this vector"},
    {"div",                 Quat_div,                 METH_O,       "Adds another vector or scalar to this vector"},
    {"dot",                 Quat_dot,                 METH_O,       "Adds another vector or scalar to this vector"},
    {"ensure_positive_sign",Quat_ensure_positive_sign,METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"angle",               Quat_angle,               METH_VARARGS, "Adds another vector or scalar to this vector"},
    {"angle_no_normalize",  Quat_angle_no_normalize,  METH_VARARGS, "Adds another vector or scalar to this vector"},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyMemberDef Quat_members[] = {
    {"w", MEMBER_TYPE, offsetof(ik_Quat, quat.q.w), 0, "W component"},
    {"x", MEMBER_TYPE, offsetof(ik_Quat, quat.q.x), 0, "X component"},
    {"y", MEMBER_TYPE, offsetof(ik_Quat, quat.q.y), 0, "Y component"},
    {"z", MEMBER_TYPE, offsetof(ik_Quat, quat.q.z), 0, "Z component"},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(QUAT_DOC, "");
PyTypeObject ik_QuatType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Quat",
    .tp_basicsize = sizeof(ik_Quat),
    .tp_dealloc = Quat_dealloc,
    .tp_repr = Quat_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = QUAT_DOC,
    .tp_methods = Quat_methods,
    .tp_members = Quat_members,
    .tp_init = Quat_init,
    .tp_new = Quat_new
};

/* ------------------------------------------------------------------------- */
int
init_ik_QuatType(void)
{
    if (PyType_Ready(&ik_QuatType) < 0)
        return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
int
quat_python_to_ik(PyObject* qpy, ikreal qik[4])
{
    if (ik_Vec3_CheckExact(qpy))
    {
        ik_Quat* q = (ik_Quat*)qpy;
        ik_quat_copy(qik, q->quat.f);
    }
    else if(PySequence_Check(qpy))
    {
        if (PySequence_Fast_GET_SIZE(qpy) != 4)
        {
            PyErr_Format(PyExc_TypeError, "Expected an array of 4 values, but got %d", PySequence_Fast_GET_SIZE(qpy));
            return -1;
        }

        PyObject* x = PySequence_Fast_GET_ITEM(qpy, 0);
        PyObject* y = PySequence_Fast_GET_ITEM(qpy, 1);
        PyObject* z = PySequence_Fast_GET_ITEM(qpy, 2);
        PyObject* w = PySequence_Fast_GET_ITEM(qpy, 3);
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
        if (!PyFloat_Check(w))
        {
            PyErr_SetString(PyExc_TypeError, "w component is not a float");
            return -1;
        }

        qik[0] = PyFloat_AsDouble(x);
        qik[1] = PyFloat_AsDouble(y);
        qik[2] = PyFloat_AsDouble(z);
        qik[3] = PyFloat_AsDouble(w);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either a ik.Quat type or an array of 4 floats");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
ik_Quat*
quat_ik_to_python(ikreal q[4])
{
    ik_Quat* qpy = (ik_Quat*)PyObject_CallObject((PyObject*)&ik_QuatType, NULL);
    if (qpy == NULL)
        return NULL;
    ik_quat_copy(qpy->quat.f, q);
    return qpy;
}
