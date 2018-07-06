#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Vec3.h"
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
static int
Quat_init(ik_Quat* self, PyObject* args, PyObject* kwds)
{
    (void)kwds;
    IKAPI.quat.set_identity(self->quat.f);
    if (!PyArg_ParseTuple(args, "|" FMT FMT FMT FMT, &self->quat.w, &self->quat.x, &self->quat.y, &self->quat.z))
        return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_set_identity(ik_Quat* self, PyObject* arg)
{
    (void)arg;
    IKAPI.quat.set_identity(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_set(ik_Quat* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_QuatType))
    {
        IKAPI.quat.set(self->quat.f, ((ik_Quat*)arg)->quat.f);
        Py_RETURN_NONE;
    }
    else if (PyArg_ParseTuple(arg, FMT FMT FMT FMT, &self->quat.w, &self->quat.x, &self->quat.y, &self->quat.z))
    {
        Py_RETURN_NONE;
    }

    return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_add(ik_Quat* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_QuatType))
        IKAPI.quat.add_quat(self->quat.f, ((ik_Quat*)arg)->quat.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        ik_quat_t other;
        other.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        IKAPI.quat.add_quat(self->quat.f, other.f);
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
Quat_mag(ik_Quat* self, PyObject* arg)
{
    (void)arg;
    return PyFloat_FromDouble(IKAPI.quat.mag(self->quat.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_conj(ik_Quat* self, PyObject* arg)
{
    (void)arg;
    IKAPI.quat.conj(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_invert_sign(ik_Quat* self, PyObject* arg)
{
    (void)arg;
    IKAPI.quat.negate(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_normalize(ik_Quat* self, PyObject* arg)
{
    (void)arg;
    IKAPI.quat.normalize(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_mul(ik_Quat* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_QuatType))
        IKAPI.quat.mul_quat(self->quat.f, ((ik_Quat*)arg)->quat.f);
    else if (PyFloat_Check(arg))
        IKAPI.quat.mul_scalar(self->quat.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        IKAPI.quat.mul_scalar(self->quat.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        ik_quat_t other;
        other.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        IKAPI.quat.mul_quat(self->quat.f, other.f);
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
Quat_div(ik_Quat* self, PyObject* arg)
{
    if (PyFloat_Check(arg))
        IKAPI.quat.div_scalar(self->quat.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        IKAPI.quat.div_scalar(self->quat.f, PyLong_AS_LONG(arg));
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Quat type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_dot(ik_Quat* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_QuatType))
        IKAPI.quat.dot(self->quat.f, ((ik_Quat*)arg)->quat.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        ik_quat_t other;
        other.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        IKAPI.quat.dot(self->quat.f, other.f);
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
Quat_normalize_sign(ik_Quat* self, PyObject* arg)
{
    (void)arg;
    IKAPI.quat.ensure_positive_sign(self->quat.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_angle(ik_Quat* self, PyObject* args)
{
    PyObject *vec1, *vec2;

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
            IKAPI.quat.angle(self->quat.f, ((ik_Vec3*)vec1)->vec.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            ik_vec3_t other;
            other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            IKAPI.quat.angle(self->quat.f, ((ik_Vec3*)vec1)->vec.f, other.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a IK.Quat type or a tuple with 4 floats");
            return NULL;
        }
    }
    else if (PySequence_Check(vec1) && PySequence_Fast_GET_SIZE(vec1) == 3)
    {
        ik_vec3_t other1;
        other1.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 0));
        other1.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 1));
        other1.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 2));
        if (PyErr_Occurred())
            return NULL;

        if (PyObject_TypeCheck(vec2, &ik_Vec3Type))
        {
            IKAPI.quat.angle(self->quat.f, other1.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            ik_vec3_t other2;
            other2.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other2.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other2.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            IKAPI.quat.angle(self->quat.f, other1.f, other2.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a IK.Quat type or a tuple with 4 floats");
            return NULL;
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either a IK.Quat type or a tuple with 4 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_angle_normalized_vectors(ik_Quat* self, PyObject* args)
{
    PyObject *vec1, *vec2;

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
            IKAPI.quat.angle_no_normalize(self->quat.f, ((ik_Vec3*)vec1)->vec.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            ik_vec3_t other;
            other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            IKAPI.quat.angle_no_normalize(self->quat.f, ((ik_Vec3*)vec1)->vec.f, other.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a IK.Quat type or a tuple with 4 floats");
            return NULL;
        }
    }
    else if (PySequence_Check(vec1) && PySequence_Fast_GET_SIZE(vec1) == 3)
    {
        ik_vec3_t other1;
        other1.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 0));
        other1.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 1));
        other1.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 2));
        if (PyErr_Occurred())
            return NULL;

        if (PyObject_TypeCheck(vec2, &ik_Vec3Type))
        {
            IKAPI.quat.angle_no_normalize(self->quat.f, other1.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            ik_vec3_t other2;
            other2.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other2.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other2.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            IKAPI.quat.angle_no_normalize(self->quat.f, other1.f, other2.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a IK.Quat type or a tuple with 4 floats");
            return NULL;
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either a IK.Quat type or a tuple with 4 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Quat_repr(ik_Quat* self)
{
    PyObject *fmt, *args, *str, *w, *x, *y, *z;
    if ((args = PyTuple_New(4)) == NULL) goto tuple_failed;
    if ((w = PyFloat_FromDouble(self->quat.w)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 0, w);
    if ((x = PyFloat_FromDouble(self->quat.x)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 1, x);
    if ((y = PyFloat_FromDouble(self->quat.y)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 2, y);
    if ((z = PyFloat_FromDouble(self->quat.z)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 3, z);
    if ((fmt = PyUnicode_FromString("IK.Quat(%f, %f, %f, %f)")) == NULL) goto fmt_failed;
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
    {"set_identity",             (PyCFunction)Quat_set_identity,             METH_NOARGS,  "Sets all components to 0.0"},
    {"set",                      (PyCFunction)Quat_set,                      METH_O,       "Copies components from another vector or tuple"},
    {"add",                      (PyCFunction)Quat_add,                      METH_O,       "Adds another vector or scalar to this vector"},
    {"mag",                      (PyCFunction)Quat_mag,                      METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"conj",                     (PyCFunction)Quat_conj,                     METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"invert_sign",              (PyCFunction)Quat_invert_sign,              METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"normalize",                (PyCFunction)Quat_normalize,                METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"mul",                      (PyCFunction)Quat_mul,                      METH_O,       "Adds another vector or scalar to this vector"},
    {"div",                      (PyCFunction)Quat_div,                      METH_O,       "Adds another vector or scalar to this vector"},
    {"dot",                      (PyCFunction)Quat_dot,                      METH_O,       "Adds another vector or scalar to this vector"},
    {"normalize_sign",           (PyCFunction)Quat_normalize_sign,           METH_NOARGS,  "Adds another vector or scalar to this vector"},
    {"angle",                    (PyCFunction)Quat_angle,                    METH_VARARGS, "Adds another vector or scalar to this vector"},
    {"angle_normalized_vectors", (PyCFunction)Quat_angle_normalized_vectors, METH_VARARGS, "Adds another vector or scalar to this vector"},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyMemberDef Quat_members[] = {
    {"w", MEMBER_TYPE, offsetof(ik_Quat, quat.w), 0, "W component"},
    {"x", MEMBER_TYPE, offsetof(ik_Quat, quat.x), 0, "X component"},
    {"y", MEMBER_TYPE, offsetof(ik_Quat, quat.y), 0, "Y component"},
    {"z", MEMBER_TYPE, offsetof(ik_Quat, quat.z), 0, "Z component"},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject ik_QuatType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "IK.Quat",                                     /* tp_name */
    sizeof(ik_Quat),                               /* tp_basicsize */
    0,                                             /* tp_itemsize */
    0,                                             /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_reserved */
    (reprfunc)Quat_repr,                           /* tp_repr */
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
    Quat_methods,                                  /* tp_methods */
    Quat_members,                                  /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    (initproc)Quat_init,                           /* tp_init */
    0,                                             /* tp_alloc */
    0                                              /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
init_ik_QuatType(void)
{
    ik_QuatType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&ik_QuatType) < 0)
        return -1;
    return 0;
}
