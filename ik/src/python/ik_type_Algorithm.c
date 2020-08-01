#include "ik/python/ik_type_Algorithm.h"
#include "ik/python/ik_helpers.h"
#include "ik/python/ik_docstrings.h"
#include "ik/algorithm.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Algorithm_dealloc(PyObject* myself)
{
    ik_Algorithm* self = (ik_Algorithm*)myself;
    IK_DECREF(self->super.attachment);
    ik_AlgorithmType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Algorithm_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Algorithm* self;
    struct ik_algorithm* algorithm;

    if ((algorithm = ik_algorithm_create("invalid")) == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate algorithm. Check the log for more info. Possibly out of memory.");
        goto alloc_algorithm_failed;
    }
    IK_INCREF(algorithm);

    self = (ik_Algorithm*)ik_AlgorithmType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    self->super.attachment = (struct ik_attachment*)algorithm;
    return (PyObject*)self;

    alloc_self_failed      : IK_DECREF(algorithm);
    alloc_algorithm_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Algorithm_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    const char* name;
#define X(upper, lower, value) int lower = -1;
    IK_ALGORITHM_FEATURES_LIST
#undef X

    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* alg = (struct ik_algorithm*)self->super.attachment;

    static char* kwds_names[] = {
        "type",
#define X(upper, lower, value) #lower,
        IK_ALGORITHM_FEATURES_LIST
#undef X
        "max_iterations",
        "tolerance",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds,
                                     "s|"
#define X(upper, lower, value)       "p"
        IK_ALGORITHM_FEATURES_LIST
#undef X
                                     "H" FMT,
                                     kwds_names,
                                     &name,
#define X(upper, lower, value)       &lower,
        IK_ALGORITHM_FEATURES_LIST
#undef X
                                     &alg->max_iterations,
                                     &alg->tolerance))
    {
        return -1;
    }

    if (ik_algorithm_set_type(alg, name) != 0)
    {
        PyErr_Format(PyExc_ValueError, "Invalid algorithm name was specified: `%s`");
        return -1;
    }

#define X(upper, lower, value)                       \
        if (lower == 1)                              \
            alg->features |= IK_ALGORITHM_##upper;   \
        else if (lower == 0)                         \
            alg->features &= ~IK_ALGORITHM_##upper;
    IK_ALGORITHM_FEATURES_LIST
#undef X

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Algorithm_gettype(PyObject* myself, void* closure)
{
    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;
    (void)closure;

    return PyUnicode_FromString(algo->type);
}
static int
Algorithm_settype(PyObject* myself, PyObject* value, void* closure)
{
    PyObject* ascii;
    int result;
    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;
    (void)closure;

    if (!PyUnicode_Check(value))
    {
        PyErr_SetString(PyExc_TypeError, "Algorithm type must be a string.");
        return -1;
    }

    if ((ascii = PyUnicode_AsASCIIString(value)) == NULL)
        return -1;

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
static PyObject*
Algorithm_gettolerance(PyObject* myself, void* closure)
{
    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;
    (void)closure;

    return PyFloat_FromDouble(algo->tolerance);
}
static int
Algorithm_settolerance(PyObject* myself, PyObject* value, void* closure)
{
    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;
    (void)closure;

    if (!PyFloat_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Tolerance must be a float");
        return -1;
    }

    algo->tolerance = PyFloat_AS_DOUBLE(value);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Algorithm_getmax_iterations(PyObject* myself, void* closure)
{
    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;
    (void)closure;

    return PyLong_FromLong(algo->max_iterations);
}
static int
Algorithm_setmax_iterations(PyObject* myself, PyObject* value, void* closure)
{
    unsigned long max_iterations;
    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;
    const unsigned long max_iteration_count = (1 << (sizeof(algo->max_iterations) * 8)) - 1;
    (void)closure;

    if (!PyLong_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Iterations must be an integer");
        return -1;
    }
    if (PyLong_AsLong(value) < 0)
    {
        PyErr_SetString(PyExc_ValueError, "Iterations must be a positive integer");
        return -1;
    }

    max_iterations = PyLong_AsUnsignedLong(value);
    if (max_iterations > max_iteration_count)
    {
        PyErr_Format(PyExc_ValueError, "Iteration count too high. Maximum iteration count is %lu", max_iteration_count);
        return -1;
    }

    algo->max_iterations = max_iterations;
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
get_feature_flag(struct ik_algorithm* algo, enum ik_algorithm_feature feature)
{
    if (algo->features & feature)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static int
set_feature_flag(struct ik_algorithm* algo, PyObject* value, enum ik_algorithm_feature feature)
{
    int is_true = PyObject_IsTrue(value);
    if (is_true == -1)
        return -1;

    if (is_true)
        algo->features |= feature;
    else
        algo->features &= ~feature;
    return 0;
}

/* ------------------------------------------------------------------------- */
#define X(upper, lower, v)                                                    \
static PyObject*                                                              \
Algorithm_get##lower(PyObject* myself, void* closure)                         \
{                                                                             \
    ik_Algorithm* self = (ik_Algorithm*)myself;                               \
    (void)closure;                                                            \
                                                                              \
    return get_feature_flag((struct ik_algorithm*)self->super.attachment,     \
                            IK_ALGORITHM_##upper);                            \
}                                                                             \
static int                                                                    \
Algorithm_set##lower(PyObject* myself, PyObject* value, void* closure)        \
{                                                                             \
    ik_Algorithm* self = (ik_Algorithm*)myself;                               \
    (void)closure;                                                            \
                                                                              \
    return set_feature_flag((struct ik_algorithm*)self->super.attachment,     \
                            value,                                            \
                            IK_ALGORITHM_##upper);                            \
}
IK_ALGORITHM_FEATURES_LIST
#undef X

/* ------------------------------------------------------------------------- */
static PyGetSetDef Algorithm_getset[] = {
    {"type",             Algorithm_gettype,           Algorithm_settype,           IK_ALGORITHM_TYPE_DOC, NULL},
#define X(upper, lower, value) \
    {#lower,             Algorithm_get##lower,        Algorithm_set##lower,        IK_ALGORITHM_##upper##_DOC, NULL},
    IK_ALGORITHM_FEATURES_LIST
#undef X
    {"tolerance",        Algorithm_gettolerance,      Algorithm_settolerance,      IK_ALGORITHM_TOLERANCE_DOC, NULL},
    {"max_iterations",   Algorithm_getmax_iterations, Algorithm_setmax_iterations, IK_ALGORITHM_MAX_ITERATIONS_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Algorithm_repr_build_arglist_list(PyObject* myself)
{
    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* algo = (struct ik_algorithm*)self->super.attachment;

    PyObject* args = PyList_New(0);
    if (args == NULL)
        return NULL;

    /* Type */
    {
        int append_result;
        PyObject* arg = PyUnicode_FromFormat("\"%s\"", algo->type);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Tolerance */
    {
        int append_result;
        PyObject* tolerance;
        PyObject* arg;

        tolerance = PyFloat_FromDouble(algo->tolerance);
        if (tolerance == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("tolerance=%R", tolerance);
        Py_DECREF(tolerance);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Max iterations */
    {
        int append_result;
        PyObject* arg = PyUnicode_FromFormat("max_iterations=%d", (int)algo->max_iterations);
        if (arg == NULL)
            goto addarg_failed;
        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Feature flags */
#define X(upper, lower, value)                                                \
    if (algo->features & IK_ALGORITHM_##upper)                                \
    {                                                                         \
        int append_result;                                                    \
        PyObject* arg = PyUnicode_FromString(#lower "=True");                 \
        if (arg == NULL)                                                      \
            goto addarg_failed;                                               \
        append_result = PyList_Append(args, arg);                             \
        Py_DECREF(arg);                                                       \
        if (append_result == -1)                                              \
            goto addarg_failed;                                               \
    }
    IK_ALGORITHM_FEATURES_LIST
#undef X

    return args;

    addarg_failed : Py_DECREF(args);
    return NULL;
}
static PyObject*
Algorithm_repr_build_arglist_string(PyObject* myself)
{
    PyObject* separator;
    PyObject* arglist;
    PyObject* string;

    separator = PyUnicode_FromString(", ");
    if (separator == NULL)
        return NULL;

    arglist = Algorithm_repr_build_arglist_list(myself);
    if (arglist == NULL)
    {
        Py_DECREF(separator);
        return NULL;
    }

    string = PyUnicode_Join(separator, arglist);
    Py_DECREF(separator);
    Py_DECREF(arglist);
    return string;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Algorithm_repr(PyObject* myself)
{
    PyObject* repr;
    PyObject* argstring = Algorithm_repr_build_arglist_string(myself);
    if (argstring == NULL)
        return NULL;

    repr = PyUnicode_FromFormat("ik.Algorithm(%U)", argstring);
    Py_DECREF(argstring);
    return repr;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Algorithm_str(PyObject* myself)
{
    return Algorithm_repr(myself);
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_AlgorithmType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Algorithm",
    .tp_basicsize = sizeof(ik_Algorithm),
    .tp_dealloc = Algorithm_dealloc,
    .tp_repr = Algorithm_repr,
    .tp_str = Algorithm_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_ALGORITHM_DOC,
    .tp_getset = Algorithm_getset,
    .tp_new = Algorithm_new,
    .tp_init = Algorithm_init
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
