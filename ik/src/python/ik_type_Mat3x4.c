#include "ik/python/ik_type_Mat3x4.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_helpers.h"
#include "ik/mat3x4.inl"
#include "ik/vec3.inl"

/* ------------------------------------------------------------------------- */
static void
Mat3x4_dealloc(PyObject* myself)
{
    ik_Mat3x4* self = (ik_Mat3x4*)myself;

    UNREF_VEC3_DATA(self->ex);
    UNREF_VEC3_DATA(self->ey);
    UNREF_VEC3_DATA(self->ez);
    UNREF_VEC3_DATA(self->et);

    Py_DECREF(self->ex);
    Py_DECREF(self->ey);
    Py_DECREF(self->ez);
    Py_DECREF(self->et);

    Py_TYPE(myself)->tp_free(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Mat3x4_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Mat3x4* self;
    ik_Vec3* ex;
    ik_Vec3* ey;
    ik_Vec3* ez;
    ik_Vec3* et;
    (void)args; (void)kwds;

    if ((ex = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL)) == NULL)
        goto alloc_ex_failed;
    if ((ey = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL)) == NULL)
        goto alloc_ey_failed;
    if ((ez = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL)) == NULL)
        goto alloc_ez_failed;
    if ((et = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL)) == NULL)
        goto alloc_et_failed;

    self = (ik_Mat3x4*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    self->ex = ex;  REF_VEC3_DATA(self->ex, &self->mat.e.ex);
    self->ey = ey;  REF_VEC3_DATA(self->ey, &self->mat.e.ey);
    self->ez = ez;  REF_VEC3_DATA(self->ez, &self->mat.e.ez);
    self->et = et;  REF_VEC3_DATA(self->et, &self->mat.e.et);

    ik_mat3x4_set_identity(self->mat.f);

    return (PyObject*)self;

    alloc_self_failed : Py_DECREF(et);
    alloc_et_failed   : Py_DECREF(ez);
    alloc_ez_failed   : Py_DECREF(ey);
    alloc_ey_failed   : Py_DECREF(ex);
    alloc_ex_failed   : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Mat3x4_set(PyObject* myself, PyObject* arg);
static int
Mat3x4_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    (void)kwds;

    assert(PyTuple_CheckExact(args));
    if (PyTuple_GET_SIZE(args) > 0)
    {
        PyObject* result = Mat3x4_set(myself, args);
        if (result == NULL)
            return -1;
        Py_DECREF(result);
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Mat3x4_set(PyObject* myself, PyObject* args)
{
    ik_Mat3x4* self = (ik_Mat3x4*)myself;
    assert(PyTuple_CheckExact(args));

    if (PyTuple_GET_SIZE(args) == 1)
    {
        PyObject* arg = PyTuple_GET_ITEM(args, 0);
        if (ik_Mat3x4_CheckExact(arg))
        {
            ik_Mat3x4* other = (ik_Mat3x4*)arg;
            ik_mat3x4_copy(self->mat.f, other->mat.f);
            Py_RETURN_NONE;
        }

        PyErr_SetString(PyExc_TypeError, "Expected a matrix");
        return NULL;
    }
    else if (PyTuple_GET_SIZE(args) == 2)
    {
        PyObject* arg1 = PyTuple_GET_ITEM(args, 0);
        PyObject* arg2 = PyTuple_GET_ITEM(args, 1);

        if (ik_Vec3_CheckExact(arg1) && ik_Quat_CheckExact(arg2))
        {
            ik_mat3x4_from_pos_rot(self->mat.f, ((ik_Vec3*)arg1)->vecref->f, ((ik_Quat*)arg2)->quatref->f);
            Py_RETURN_NONE;
        }

        PyErr_SetString(PyExc_TypeError, "Expected a position and rotation (ik.Vec3() and ik.Quat())");
        return NULL;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a matrix");
    return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Mat3x4_dup(PyObject* other)
{
    PyObject* copy;

    assert(ik_Mat3x4_CheckExact(other));

    PyObject* args = PyTuple_New(1);
    if (args == NULL)
        return NULL;

    Py_INCREF(other);
    PyTuple_SET_ITEM(args, 0, other);  /* ref steal */

    copy = PyObject_CallObject((PyObject*)&ik_Mat3x4Type, args);
    Py_DECREF(args);
    return copy;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Mat3x4_inplace_mul(PyObject* self, PyObject* arg)
{
    if (ik_Mat3x4_CheckExact(arg))
    {
        ik_mat3x4_mul_mat3x4(((ik_Mat3x4*)self)->mat.f, ((ik_Mat3x4*)arg)->mat.f);
        return Py_INCREF(self), self;
    }

    if (PyFloat_Check(arg))
    {
        ik_mat3x4_mul_scalar(((ik_Mat3x4*)self)->mat.f, PyFloat_AS_DOUBLE(arg));
        return Py_INCREF(self), self;
    }
    else if (PyLong_Check(arg))
    {
        double v = PyLong_AsDouble(arg);
        if (v == -1.0 && PyErr_Occurred())
            return NULL;
        ik_mat3x4_mul_scalar(((ik_Mat3x4*)self)->mat.f, v);
        return Py_INCREF(self), self;
    }

    PyErr_SetString(PyExc_TypeError, "Right operand must be a ik.Mat3x4() or a scalar value");
    return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Mat3x4_mul(PyObject* myself, PyObject* arg)
{
    if (ik_Vec3_CheckExact(arg))
    {
        PyObject* result;
        PyObject* args = PyTuple_New(1);
        if (args == NULL)
            return NULL;

        Py_INCREF(arg);
        PyTuple_SET_ITEM(args, 0, arg);  /* ref steal */

        result = PyObject_CallObject((PyObject*)&ik_Vec3Type, args);
        Py_DECREF(args);
        if (result == NULL)
            return NULL;

        ik_mat3x4_mul_vec3(((ik_Mat3x4*)myself)->mat.f, ((ik_Vec3*)result)->vecref->f);
        return result;
    }
    else
    {
        PyObject* op_result;
        PyObject* result = Mat3x4_dup(myself);
        if (result == NULL)
            return NULL;
        if ((op_result = Mat3x4_inplace_mul(result, arg)) == NULL)
            return Py_DECREF(result), NULL;
        Py_DECREF(op_result);
        return result;
    }
}

/* ------------------------------------------------------------------------- */
static PyObject*
Mat3x4_repr(PyObject* myself)
{
    PyObject *m00, *m01, *m02, *m03;
    PyObject *m10, *m11, *m12, *m13;
    PyObject *m20, *m21, *m22, *m23;
    PyObject* str = NULL;
    ik_Mat3x4* self = (ik_Mat3x4*)myself;

    if ((m00 = PyFloat_FromDouble(self->mat.m.m00)) == NULL) goto m00_failed;
    if ((m01 = PyFloat_FromDouble(self->mat.m.m01)) == NULL) goto m01_failed;
    if ((m02 = PyFloat_FromDouble(self->mat.m.m02)) == NULL) goto m02_failed;
    if ((m03 = PyFloat_FromDouble(self->mat.m.m03)) == NULL) goto m03_failed;
    if ((m10 = PyFloat_FromDouble(self->mat.m.m10)) == NULL) goto m10_failed;
    if ((m11 = PyFloat_FromDouble(self->mat.m.m11)) == NULL) goto m11_failed;
    if ((m12 = PyFloat_FromDouble(self->mat.m.m12)) == NULL) goto m12_failed;
    if ((m13 = PyFloat_FromDouble(self->mat.m.m13)) == NULL) goto m13_failed;
    if ((m20 = PyFloat_FromDouble(self->mat.m.m20)) == NULL) goto m20_failed;
    if ((m21 = PyFloat_FromDouble(self->mat.m.m21)) == NULL) goto m21_failed;
    if ((m22 = PyFloat_FromDouble(self->mat.m.m22)) == NULL) goto m22_failed;
    if ((m23 = PyFloat_FromDouble(self->mat.m.m23)) == NULL) goto m23_failed;
    str = PyUnicode_FromFormat("ik.Mat3x4(\n"
        "    %S, %S, %S, %S\n"
        "    %S, %S, %S, %S\n"
        "    %S, %S, %S, %S)",
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23);

                 Py_DECREF(m23);
    m23_failed : Py_DECREF(m22);
    m22_failed : Py_DECREF(m21);
    m21_failed : Py_DECREF(m20);
    m20_failed : Py_DECREF(m13);
    m13_failed : Py_DECREF(m12);
    m12_failed : Py_DECREF(m11);
    m11_failed : Py_DECREF(m10);
    m10_failed : Py_DECREF(m03);
    m03_failed : Py_DECREF(m02);
    m02_failed : Py_DECREF(m01);
    m01_failed : Py_DECREF(m00);
    m00_failed : return str;
}

/* ------------------------------------------------------------------------- */
static PyNumberMethods Mat3x4_as_number = {
    .nb_multiply = Mat3x4_mul,
    .nb_inplace_multiply = Mat3x4_inplace_mul
};

/* ------------------------------------------------------------------------- */
static PyMethodDef Mat3x4_methods[] = {
    {NULL}
};

/* ------------------------------------------------------------------------- */
#define GETSET_BASIS_VECTOR(basis)                                            \
    static PyObject*                                                          \
    Mat3x4_get##basis(PyObject* myself, void* closure)                        \
    {                                                                         \
        ik_Mat3x4* self = (ik_Mat3x4*)myself;                                 \
        (void)closure;                                                        \
        return Py_INCREF(self->basis), (PyObject*)self->basis;                \
    }                                                                         \
    static int                                                                \
    Mat3x4_set##basis(PyObject* myself, PyObject* value, void* closure)       \
    {                                                                         \
        ik_Mat3x4* self = (ik_Mat3x4*)myself;                                 \
        (void)closure;                                                        \
        if (!ik_Vec3_CheckExact(value))                                       \
        {                                                                     \
            PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type for basis vector"); \
            return -1;                                                        \
        }                                                                     \
                                                                              \
        ASSIGN_VEC3(self->basis, (ik_Vec3*)value);                            \
        return 0;                                                             \
    }
GETSET_BASIS_VECTOR(ex)
GETSET_BASIS_VECTOR(ey)
GETSET_BASIS_VECTOR(ez)
GETSET_BASIS_VECTOR(et)
#undef GETSET_BASIS_VECTOR

/* ------------------------------------------------------------------------- */
#define GETSET_COMPONENT(comp)                                                \
    static PyObject*                                                          \
    Mat3x4_get##comp(PyObject* myself, void* closure)                         \
    {                                                                         \
        ik_Mat3x4* self = (ik_Mat3x4*)myself;                                 \
        (void)closure;                                                        \
        return PyFloat_FromDouble(self->mat.m.comp);                          \
    }                                                                         \
    static int                                                                \
    Mat3x4_set##comp(PyObject* myself, PyObject* value, void* closure)        \
    {                                                                         \
        ik_Mat3x4* self = (ik_Mat3x4*)myself;                                 \
        (void)closure;                                                        \
                                                                              \
        if (PyFloat_Check(value))                                             \
            self->mat.m.comp = PyFloat_AS_DOUBLE(value);                      \
        else if (PyLong_Check(value))                                         \
        {                                                                     \
            double v = PyLong_AsDouble(value);                                \
            if (v == -1.0 && PyErr_Occurred())                                \
                return -1;                                                    \
            self->mat.m.comp = v;                                             \
        }                                                                     \
        else                                                                  \
        {                                                                     \
            PyErr_SetString(PyExc_TypeError, "Expected a value");             \
            return -1;                                                        \
        }                                                                     \
                                                                              \
        return 0;                                                             \
    }
GETSET_COMPONENT(m00)
GETSET_COMPONENT(m01)
GETSET_COMPONENT(m02)
GETSET_COMPONENT(m03)
GETSET_COMPONENT(m10)
GETSET_COMPONENT(m11)
GETSET_COMPONENT(m12)
GETSET_COMPONENT(m13)
GETSET_COMPONENT(m20)
GETSET_COMPONENT(m21)
GETSET_COMPONENT(m22)
GETSET_COMPONENT(m23)
#undef GETSET_COMPONENT

/* ------------------------------------------------------------------------- */
static PyGetSetDef Mat3x4_getsetters[] = {
    {"ex",  Mat3x4_getex,  Mat3x4_setex,  "X basis vector"},
    {"ey",  Mat3x4_getey,  Mat3x4_setey,  "Y basis vector"},
    {"ez",  Mat3x4_getez,  Mat3x4_setez,  "Z basis vector"},
    {"et",  Mat3x4_getet,  Mat3x4_setet,  "T basis vector"},
    {"m00", Mat3x4_getm00, Mat3x4_setm00, "Value at row 0, column 0"},
    {"m01", Mat3x4_getm01, Mat3x4_setm01, "Value at row 0, column 1"},
    {"m02", Mat3x4_getm02, Mat3x4_setm02, "Value at row 0, column 2"},
    {"m03", Mat3x4_getm03, Mat3x4_setm03, "Value at row 0, column 3"},
    {"m10", Mat3x4_getm10, Mat3x4_setm10, "Value at row 1, column 0"},
    {"m11", Mat3x4_getm11, Mat3x4_setm11, "Value at row 1, column 1"},
    {"m12", Mat3x4_getm12, Mat3x4_setm12, "Value at row 1, column 2"},
    {"m13", Mat3x4_getm13, Mat3x4_setm13, "Value at row 1, column 3"},
    {"m20", Mat3x4_getm20, Mat3x4_setm20, "Value at row 2, column 0"},
    {"m21", Mat3x4_getm21, Mat3x4_setm21, "Value at row 2, column 1"},
    {"m22", Mat3x4_getm22, Mat3x4_setm22, "Value at row 2, column 2"},
    {"m23", Mat3x4_getm23, Mat3x4_setm23, "Value at row 2, column 3"},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(MAT3X4_DOC, "");
PyTypeObject ik_Mat3x4Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Mat3x4",
    .tp_basicsize = sizeof(ik_Mat3x4),
    .tp_dealloc = Mat3x4_dealloc,
    .tp_repr = Mat3x4_repr,
    .tp_as_number = &Mat3x4_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = MAT3X4_DOC,
    .tp_methods = Mat3x4_methods,
    .tp_getset = Mat3x4_getsetters,
    .tp_init = Mat3x4_init,
    .tp_new = Mat3x4_new
};

/* ------------------------------------------------------------------------- */
int
init_ik_Mat3x4Type(void)
{
    if (PyType_Ready(&ik_Mat3x4Type) < 0)
        return -1;
    return 0;
}
