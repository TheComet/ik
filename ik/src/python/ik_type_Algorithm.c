#include "ik/python/ik_type_Algorithm.h"
#include "ik/algorithm.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Algorithm_dealloc(ik_Algorithm* self)
{
    IK_DECREF(self->super.attachment);
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Algorithm_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Algorithm* self;
    struct ik_algorithm* algorithm;
    const char* name;

    static char* kwds_names[] = {
        "type",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwds_names, &name))
        return NULL;

    if ((algorithm = ik_algorithm_create(name)) == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate algorithm. Check the log for more info. Possibly out of memory, or the name is too long.");
        goto alloc_algorithm_failed;
    }
    IK_INCREF(algorithm);

    self = (ik_Algorithm*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    self->super.attachment = (struct ik_attachment*)algorithm;

    return (PyObject*)self;

    alloc_self_failed      : IK_DECREF(algorithm);
    alloc_algorithm_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(ALGORITHM_TYPE_DOC,
"");
static PyObject*
Algorithm_gettype(ik_Algorithm* self, void* closure)
{
    struct ik_algorithm* algo;
    (void)closure;

    algo = (struct ik_algorithm*)self->super.attachment;
    return PyUnicode_FromString(algo->type);
}
static int
Algorithm_settype(ik_Algorithm* self, PyObject* value, void* closure)
{
    struct ik_algorithm* algo;
    PyObject* ascii;
    int result;
    (void)closure;

    if (!PyUnicode_Check(value))
    {
        PyErr_SetString(PyExc_TypeError, "Algorithm type must be a string.");
        return -1;
    }

    if ((ascii = PyUnicode_AsASCIIString(value)) == NULL)
        return -1;

    algo = (struct ik_algorithm*)self->super.attachment;
    result = ik_algorithm_set_type(algo, PyBytes_AS_STRING(ascii));
    Py_DECREF(ascii);

    if (result != 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to set algorithm type. Check the log for more info. It's possiblee the name is too long.");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(ALGORITHM_TOLERANCE_DOC, "");
static PyObject*
Algorithm_gettolerance(ik_Algorithm* self, void* closure)
{
    (void)closure;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;
    return PyFloat_FromDouble(algo->tolerance);
}
static int
Algorithm_settolerance(ik_Algorithm* self, PyObject* value, void* closure)
{
    (void)closure;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;

    if (!PyFloat_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Tolerance must be a float");
        return -1;
    }

    algo->tolerance = PyFloat_AS_DOUBLE(value);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(ALGORITHM_MAX_ITERATIONS_DOC, "");
static PyObject*
Algorithm_getmax_iterations(ik_Algorithm* self, void* closure)
{
    (void)closure;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;
    return PyLong_FromLong(algo->max_iterations);
}
static int
Algorithm_setmax_iterations(ik_Algorithm* self, PyObject* value, void* closure)
{
    (void)closure;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;

    if (!PyLong_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Tolerance must be a float");
        return -1;
    }

    algo->max_iterations = PyLong_AS_LONG(value);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Algorithm_getset[] = {
    {"type",           (getter)Algorithm_gettype,           (setter)Algorithm_settype,           ALGORITHM_TYPE_DOC, NULL},
    {"tolerance",      (getter)Algorithm_gettolerance,      (setter)Algorithm_settolerance,      ALGORITHM_TOLERANCE_DOC, NULL},
    {"max_iterations", (getter)Algorithm_getmax_iterations, (setter)Algorithm_setmax_iterations, ALGORITHM_MAX_ITERATIONS_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(ALGORITHM_DOC, "");
PyTypeObject ik_AlgorithmType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Algorithm",
    .tp_basicsize = sizeof(ik_Algorithm),
    .tp_dealloc = (destructor)Algorithm_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = ALGORITHM_DOC,
    .tp_getset = Algorithm_getset,
    .tp_new = Algorithm_new
};

/* ------------------------------------------------------------------------- */
int
init_ik_AlgorithmType(void)
{
    ik_AlgorithmType.tp_base = &ik_AttachmentType;
    if (PyType_Ready(&ik_AlgorithmType) < 0)
        return -1;
    return 0;
}
