#include "ik/python/ik_module_node.h"
#include "structmember.h"


/* ------------------------------------------------------------------------- */
static int
Node_init(ik_Node* self, PyObject* args, PyObject* kwds)
{
    (void)kwds, (void)args, (void)self;
    return 0;
}

#if 0

/* ------------------------------------------------------------------------- */
static PyObject*
Node_set_identity(ik_Node* self, PyObject* arg)
{
    (void)arg;
    node_set_identity(self->node.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_set(ik_Node* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_NodeType))
    {
        node_set(self->node.f, ((ik_Node*)arg)->node.f);
        Py_RETURN_NONE;
    }
    else if (PyArg_ParseTuple(arg, FMT FMT FMT FMT, &self->node.w, &self->node.x, &self->node.y, &self->node.z))
    {
        Py_RETURN_NONE;
    }

    return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_add(ik_Node* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_NodeType))
        node_add_node(self->node.f, ((ik_Node*)arg)->node.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        node_t other;
        other.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        node_add_node(self->node.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Node type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_mag(ik_Node* self, PyObject* arg)
{
    (void)arg;
    return PyFloat_FromDouble(node_mag(self->node.f));
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_conj(ik_Node* self, PyObject* arg)
{
    (void)arg;
    node_conj(self->node.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_invert_sign(ik_Node* self, PyObject* arg)
{
    (void)arg;
    node_invert_sign(self->node.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_normalize(ik_Node* self, PyObject* arg)
{
    (void)arg;
    node_normalize(self->node.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_mul(ik_Node* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_NodeType))
        node_mul_node(self->node.f, ((ik_Node*)arg)->node.f);
    else if (PyFloat_Check(arg))
        node_mul_scalar(self->node.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        node_mul_scalar(self->node.f, PyLong_AS_LONG(arg));
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        node_t other;
        other.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        node_mul_node(self->node.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Node type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_div(ik_Node* self, PyObject* arg)
{
    if (PyFloat_Check(arg))
        node_div_scalar(self->node.f, PyFloat_AS_DOUBLE(arg));
    else if (PyLong_Check(arg))
        node_div_scalar(self->node.f, PyLong_AS_LONG(arg));
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Node type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_dot(ik_Node* self, PyObject* arg)
{
    if (PyObject_TypeCheck(arg, &ik_NodeType))
        node_dot(self->node.f, ((ik_Node*)arg)->node.f);
    else if (PySequence_Check(arg) && PySequence_Fast_GET_SIZE(arg) == 4)
    {
        node_t other;
        other.w = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 0));
        other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 1));
        other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 2));
        other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(arg, 3));
        if (PyErr_Occurred())
            return NULL;
        node_dot(self->node.f, other.f);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either another Node type, a scalar, or a tuple of 3 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_normalize_sign(ik_Node* self, PyObject* arg)
{
    (void)arg;
    node_normalize_sign(self->node.f);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_angle_unnormalized(ik_Node* self, PyObject* args)
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
            node_angle_unnormalized(self->node.f, ((ik_Vec3*)vec1)->vec.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            vec3_t other;
            other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            node_angle_unnormalized(self->node.f, ((ik_Vec3*)vec1)->vec.f, other.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a ik.Node type or a tuple with 4 floats");
            return NULL;
        }
    }
    else if (PySequence_Check(vec1) && PySequence_Fast_GET_SIZE(vec1) == 3)
    {
        vec3_t other1;
        other1.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 0));
        other1.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 1));
        other1.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 2));
        if (PyErr_Occurred())
            return NULL;

        if (PyObject_TypeCheck(vec2, &ik_Vec3Type))
        {
            node_angle_unnormalized(self->node.f, other1.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            vec3_t other2;
            other2.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other2.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other2.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            node_angle_unnormalized(self->node.f, other1.f, other2.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a ik.Node type or a tuple with 4 floats");
            return NULL;
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either a ik.Node type or a tuple with 4 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_angle(ik_Node* self, PyObject* args)
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
            node_angle(self->node.f, ((ik_Vec3*)vec1)->vec.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            vec3_t other;
            other.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            node_angle(self->node.f, ((ik_Vec3*)vec1)->vec.f, other.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a ik.Node type or a tuple with 4 floats");
            return NULL;
        }
    }
    else if (PySequence_Check(vec1) && PySequence_Fast_GET_SIZE(vec1) == 3)
    {
        vec3_t other1;
        other1.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 0));
        other1.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 1));
        other1.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec1, 2));
        if (PyErr_Occurred())
            return NULL;

        if (PyObject_TypeCheck(vec2, &ik_Vec3Type))
        {
            node_angle(self->node.f, other1.f, ((ik_Vec3*)vec2)->vec.f);
        }
        else if (PySequence_Check(vec2) && PySequence_Fast_GET_SIZE(vec2) == 3)
        {
            vec3_t other2;
            other2.x = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 0));
            other2.y = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 1));
            other2.z = PyFloat_AsDouble(PySequence_Fast_GET_ITEM(vec2, 2));
            if (PyErr_Occurred())
                return NULL;
            node_angle(self->node.f, other1.f, other2.f);
        }
        else
        {
            PyErr_SetString(PyExc_TypeError, "Expected either a ik.Node type or a tuple with 4 floats");
            return NULL;
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Expected either a ik.Node type or a tuple with 4 floats");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_repr(ik_Node* self)
{
    PyObject *fmt, *args, *str, *w, *x, *y, *z;
    if ((args = PyTuple_New(4)) == NULL) goto tuple_failed;
    if ((w = PyFloat_FromDouble(self->node.w)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 0, w);
    if ((x = PyFloat_FromDouble(self->node.x)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 1, x);
    if ((y = PyFloat_FromDouble(self->node.y)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 2, y);
    if ((z = PyFloat_FromDouble(self->node.z)) == NULL) goto insert_failed;
    PyTuple_SET_ITEM(args, 3, z);
    if ((fmt = PyUnicode_FromString("ik.Node(%f, %f, %f, %f)")) == NULL) goto fmt_failed;
    if ((str = PyUnicode_Format(fmt, args)) == NULL) goto str_failed;

    Py_DECREF(fmt);
    Py_DECREF(args);
    return str;

    str_failed    : Py_DECREF(fmt);
    fmt_failed    :
    insert_failed : Py_DECREF(args);
    tuple_failed  : return NULL;
}

#endif

/* ------------------------------------------------------------------------- */
static PyMethodDef Node_methods[] = {
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyMemberDef Node_members[] = {
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Node_repr(ik_Node* self)
{
    (void)self;
    PyObject *fmt, *args, *str;
    if ((args = PyTuple_New(0)) == NULL) goto tuple_failed;
    if ((fmt = PyUnicode_FromString("ik.Node()")) == NULL) goto fmt_failed;
    if ((str = PyUnicode_Format(fmt, args)) == NULL) goto str_failed;

    Py_DECREF(fmt);
    Py_DECREF(args);
    return str;

    str_failed    : Py_DECREF(fmt);
    fmt_failed    : Py_DECREF(args);
    tuple_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_NodeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ik.Node",                                     /* tp_name */
    sizeof(ik_Node),                               /* tp_basicsize */
    0,                                             /* tp_itemsize */
    0,                                             /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_reserved */
    (reprfunc)Node_repr,                           /* tp_repr */
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
    Node_methods,                                  /* tp_methods */
    Node_members,                                  /* tp_members */
    0,                                             /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    (initproc)Node_init,                           /* tp_init */
    0,                                             /* tp_alloc */
    0                                              /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
init_ik_NodeType(void)
{
    ik_NodeType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&ik_NodeType) < 0)
        return -1;
    return 0;
}
