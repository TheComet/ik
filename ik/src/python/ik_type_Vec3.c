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

/* Macro and helper that convert PyObject obj to a C double and store
   the value in dbl.  If conversion to double raises an exception, obj is
   set to NULL, and the function invoking this macro returns NULL.  If
   obj is not of float or int type, Py_NotImplemented is incref'ed,
   stored in obj, and returned from the function invoking this macro.
*/
#define CONVERT_TO_DOUBLE(obj, dbl)                     \
    if (PyFloat_Check(obj))                             \
        dbl = PyFloat_AS_DOUBLE(obj);                   \
    else if (convert_to_double(&(obj), &(dbl)) < 0)     \
        return obj;

static int
convert_to_double(PyObject **v, double *dbl)
{
    PyObject *obj = *v;

    if (PyLong_Check(obj)) {
        *dbl = PyLong_AsDouble(obj);
        if (*dbl == -1.0 && PyErr_Occurred()) {
            *v = NULL;
            return -1;
        }
    }
    else {
        Py_INCREF(Py_NotImplemented);
        *v = Py_NotImplemented;
        return -1;
    }
    return 0;
}

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
Vec3_set(PyObject* myself, PyObject* args);
static int
Vec3_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    (void)kwds;

    assert(PyTuple_CheckExact(args));
    if (PyTuple_GET_SIZE(args) > 0)
    {
        PyObject* result = Vec3_set(myself, args);
        if (result == NULL)
            return -1;
        Py_DECREF(result);
    }

    return 0;
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
Vec3_set(PyObject* myself, PyObject* args)
{
    double x, y, z, w;
    union ik_quat rot;
    ik_Vec3* self = (ik_Vec3*)myself;
    assert(PyTuple_CheckExact(args));

    if (PyTuple_GET_SIZE(args) == 1)
    {
        PyObject* arg = PyTuple_GET_ITEM(args, 0);
        if (ik_Vec3_CheckExact(arg))
        {
            ik_Vec3* other = (ik_Vec3*)arg;
            ik_vec3_copy(self->vec.f, other->vec.f);
            Py_RETURN_NONE;
        }
        else if (ik_Quat_CheckExact(arg))
        {
            ik_vec3_set(self->vec.f, 0, 0, 1);
            ik_vec3_rotate_quat(self->vec.f, ((ik_Quat*)arg)->quat.f);
            Py_RETURN_NONE;
        }

        arg = PySequence_Tuple(arg);
        if (arg == NULL)
            return NULL;
        if (PyTuple_GET_SIZE(arg) == 3)
        {
            if (!PyArg_ParseTuple(arg, "ddd", &x, &y, &z))
            {
                Py_DECREF(arg);
                return NULL;
            }
            ik_vec3_set(self->vec.f, x, y, z);
            Py_RETURN_NONE;
        }
        else
        {
            if (!PyArg_ParseTuple(arg, "dddd", &x, &y, &z, &w))
            {
                Py_DECREF(arg);
                return NULL;
            }
            ik_quat_set(rot.f, x, y, z, w);
            ik_vec3_set(self->vec.f, 0, 0, 1);
            ik_vec3_rotate_quat(self->vec.f, rot.f);
        }

        Py_DECREF(arg);
        Py_RETURN_NONE;
    }
    else if (PyTuple_GET_SIZE(args) == 3)
    {
        if (!PyArg_ParseTuple(args, "ddd", &x, &y, &z))
            return NULL;
        ik_vec3_set(self->vec.f, x, y, z);
        Py_RETURN_NONE;
    }

    if (!PyArg_ParseTuple(args, "dddd", &x, &y, &z, &w))
        return NULL;
    ik_quat_set(rot.f, x, y, z, w);
    ik_vec3_set(self->vec.f, 0, 0, 1);
    ik_vec3_rotate_quat(self->vec.f, rot.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_dup(PyObject* other)
{
    PyObject* copy;

    assert(ik_Vec3_CheckExact(other));

    PyObject* args = PyTuple_New(1);
    if (args == NULL)
        return NULL;

    Py_INCREF(other);
    PyTuple_SET_ITEM(args, 0, other);  /* ref steal */

    copy = PyObject_CallObject((PyObject*)&ik_Vec3Type, args);
    Py_DECREF(args);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_inplace_add(PyObject* myself, PyObject* arg)
{
    if (ik_Vec3_CheckExact(arg))
    {
        ik_vec3_add_vec3(((ik_Vec3*)myself)->vec.f, ((ik_Vec3*)arg)->vec.f);
    }
    else
    {
        double v;
        CONVERT_TO_DOUBLE(arg, v)
        ik_vec3_add_scalar(((ik_Vec3*)myself)->vec.f, v);
    }

    return Py_INCREF(myself), myself;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_inplace_sub(PyObject* myself, PyObject* arg)
{
    if (ik_Vec3_CheckExact(arg))
    {
        ik_vec3_sub_vec3(((ik_Vec3*)myself)->vec.f, ((ik_Vec3*)arg)->vec.f);
    }
    else
    {
        double v;
        CONVERT_TO_DOUBLE(arg, v)
        ik_vec3_sub_scalar(((ik_Vec3*)myself)->vec.f, v);
    }

    return Py_INCREF(myself), myself;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_inplace_mul(PyObject* myself, PyObject* arg)
{
    if (ik_Vec3_CheckExact(arg))
    {
        ik_vec3_mul_vec3(((ik_Vec3*)myself)->vec.f, ((ik_Vec3*)arg)->vec.f);
    }
    else if (ik_Quat_CheckExact(arg))
    {
        ik_vec3_rotate_quat(((ik_Vec3*)myself)->vec.f, ((ik_Quat*)arg)->quat.f);
    }
    else
    {
        double v;
        CONVERT_TO_DOUBLE(arg, v)
        ik_vec3_mul_scalar(((ik_Vec3*)myself)->vec.f, v);
    }

    return Py_INCREF(myself), myself;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_inplace_div(PyObject* myself, PyObject* arg)
{
    if (ik_Vec3_CheckExact(arg))
    {
        ik_vec3_div_vec3(((ik_Vec3*)myself)->vec.f, ((ik_Vec3*)arg)->vec.f);
    }
    else
    {
        double v;
        CONVERT_TO_DOUBLE(arg, v)
        ik_vec3_div_scalar(((ik_Vec3*)myself)->vec.f, v);
    }

    return Py_INCREF(myself), myself;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_inplace_negate(PyObject* myself, PyObject* none)
{
    ikreal* v = ((ik_Vec3*)myself)->vec.f;
    (void)none;

    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];

    return Py_INCREF(myself), myself;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_inplace_normalize(PyObject* myself, PyObject* none)
{
    (void)none;
    ik_vec3_normalize(((ik_Vec3*)myself)->vec.f);
    return Py_INCREF(myself), myself;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_inplace_cross(PyObject* myself, PyObject* arg)
{
    if (!ik_Vec3_CheckExact(arg))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type");
        return NULL;
    }

    ik_vec3_cross(((ik_Vec3*)myself)->vec.f, ((ik_Vec3*)arg)->vec.f);
    return Py_INCREF(myself), myself;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_inplace_rotate(PyObject* myself, PyObject* arg)
{
    if (!ik_Quat_CheckExact(arg))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Quat() type");
        return NULL;
    }

    ik_vec3_rotate_quat(((ik_Vec3*)myself)->vec.f, ((ik_Quat*)arg)->quat.f);
    return Py_INCREF(myself), myself;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_add(PyObject* myself, PyObject* arg)
{
    PyObject* op_result;
    PyObject* copy = Vec3_dup(myself);
    if (copy == NULL)
        return NULL;
    if ((op_result = Vec3_inplace_add(copy, arg)) == NULL)
        return Py_DECREF(copy), NULL;
    Py_DECREF(op_result);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_sub(PyObject* myself, PyObject* arg)
{
    PyObject* op_result;
    PyObject* copy = Vec3_dup(myself);
    if (copy == NULL)
        return NULL;
    if ((op_result = Vec3_inplace_sub(copy, arg)) == NULL)
        return Py_DECREF(copy), NULL;
    Py_DECREF(op_result);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_mul(PyObject* myself, PyObject* arg)
{
    PyObject* op_result;
    PyObject* copy = Vec3_dup(myself);
    if (copy == NULL)
        return NULL;
    if ((op_result = Vec3_inplace_mul(copy, arg)) == NULL)
        return Py_DECREF(copy), NULL;
    Py_DECREF(op_result);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_div(PyObject* myself, PyObject* arg)
{
    PyObject* op_result;
    PyObject* copy = Vec3_dup(myself);
    if (copy == NULL)
        return NULL;
    if ((op_result = Vec3_inplace_div(copy, arg)) == NULL)
        return Py_DECREF(copy), NULL;
    Py_DECREF(op_result);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_negate(PyObject* myself)
{
    PyObject* op_result;
    PyObject* copy = Vec3_dup(myself);
    if (copy == NULL)
        return NULL;
    if ((op_result = Vec3_inplace_negate(copy, NULL)) == NULL)
        return Py_DECREF(copy), NULL;
    Py_DECREF(op_result);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_vec3(PyObject* myself)
{
    return Vec3_dup(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_length_squared(PyObject* myself, PyObject* none)
{
    (void)none;
    return PyFloat_FromDouble(ik_vec3_length_squared(((ik_Vec3*)myself)->vec.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_length(PyObject* myself)
{
    return PyFloat_FromDouble(ik_vec3_length(((ik_Vec3*)myself)->vec.f));
}
static PyObject*
Vec3_length_meth(PyObject* myself, PyObject* none)
{
    (void)none;
    return PyFloat_FromDouble(ik_vec3_length(((ik_Vec3*)myself)->vec.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_dot(PyObject* myself, PyObject* arg)
{
    if (!ik_Vec3_CheckExact(arg))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type");
        return NULL;
    }

    return PyFloat_FromDouble(ik_vec3_dot(((ik_Vec3*)myself)->vec.f, ((ik_Vec3*)arg)->vec.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_normalize(PyObject* myself, PyObject* none)
{
    PyObject* op_result;
    PyObject* copy = Vec3_dup(myself);
    if (copy == NULL)
        return NULL;
    if ((op_result = Vec3_inplace_normalize(copy, none)) == NULL)
        return Py_DECREF(copy), NULL;
    Py_DECREF(op_result);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_cross(PyObject* myself, PyObject* arg)
{
    PyObject* op_result;
    PyObject* copy = Vec3_dup(myself);
    if (copy == NULL)
        return NULL;
    if ((op_result = Vec3_inplace_cross(copy, arg)) == NULL)
        return Py_DECREF(copy), NULL;
    Py_DECREF(op_result);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_rotate(PyObject* myself, PyObject* arg)
{
    PyObject* op_result;
    PyObject* copy = Vec3_dup(myself);
    if (copy == NULL)
        return NULL;
    if ((op_result = Vec3_inplace_rotate(copy, arg)) == NULL)
        return Py_DECREF(copy), NULL;
    Py_DECREF(op_result);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Vec3_repr(PyObject* myself)
{
    PyObject *x, *y, *z;
    PyObject* str = NULL;
    ik_Vec3* self = (ik_Vec3*)myself;

    if ((x = PyFloat_FromDouble(self->vec.v.x)) == NULL) goto x_failed;
    if ((y = PyFloat_FromDouble(self->vec.v.y)) == NULL) goto y_failed;
    if ((z = PyFloat_FromDouble(self->vec.v.z)) == NULL) goto z_failed;
    str = PyUnicode_FromFormat("ik.Vec3(%S, %S, %S)", x, y, z);

               Py_DECREF(z);
    z_failed : Py_DECREF(y);
    y_failed : Py_DECREF(x);
    x_failed : return str;
}

/* ------------------------------------------------------------------------- */
static PyNumberMethods Vec3_as_number = {
    .nb_add = Vec3_add,
    .nb_subtract = Vec3_sub,
    .nb_multiply = Vec3_mul,
    .nb_true_divide = Vec3_div,
    .nb_negative = Vec3_negate,
    .nb_positive = Vec3_vec3,
    .nb_absolute = Vec3_length,
    .nb_inplace_add = Vec3_inplace_add,
    .nb_inplace_subtract = Vec3_inplace_sub,
    .nb_inplace_multiply = Vec3_inplace_mul,
    .nb_inplace_true_divide = Vec3_inplace_div
};

/* ------------------------------------------------------------------------- */
static PyMethodDef Vec3_methods[] = {
    {"set_zero",       Vec3_set_zero,          METH_NOARGS,  "Sets all components to 0.0"},
    {"set",            Vec3_set,               METH_VARARGS, "Copies components from another vector or tuple"},
    {"add",            Vec3_inplace_add,       METH_O,       "Adds another vector or scalar to this vector"},
    {"sub",            Vec3_inplace_sub,       METH_O,       "Subtracts another vector or scalar from this vector"},
    {"mul",            Vec3_inplace_mul,       METH_O,       "Multiplies another vector or scalar to this vector"},
    {"div",            Vec3_inplace_div,       METH_O,       "Divides another vector or scalar to this vector"},
    {"length_squared", Vec3_length_squared,    METH_NOARGS,  "Returns the squared length of the vector"},
    {"length",         Vec3_length_meth,       METH_NOARGS,  "Returns the length of the vector"},
    {"dot",            Vec3_dot,               METH_O,       "Calculate dot product of two vectors"},
    {"normalize",      Vec3_inplace_normalize, METH_NOARGS,  "Normalizes the vector"},
    {"normalized",     Vec3_normalize,         METH_NOARGS,  ""},
    {"cross",          Vec3_inplace_cross,     METH_O,       "Calculate cross product of two vectors"},
    {"crossed",        Vec3_cross,             METH_O,       "Calculate cross product of two vectors"},
    {"rotate",         Vec3_inplace_rotate,    METH_O,       "Rotate a vector by a quaternion"},
    {"rotated",        Vec3_rotate,            METH_O,       "Rotate a vector by a quaternion"},
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
    .tp_as_number = &Vec3_as_number,
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
ik_Vec3*
vec3_ik_to_python(ikreal v[3])
{
    ik_Vec3* vpy = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL);
    if (vpy == NULL)
        return NULL;
    ik_vec3_copy(vpy->vec.f, v);
    return vpy;
}
