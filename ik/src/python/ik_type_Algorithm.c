#include "ik/python/ik_type_Algorithm.h"
#include "ik/algorithm.h"
#include "structmember.h"

#if defined(IK_PRECISION_DOUBLE) || defined(IK_PRECISION_LONG_DOUBLE)
#   define FMT "d"
#elif defined(IK_PRECISION_FLOAT)
#   define FMT "f"
#else
#   error Dont know how to wrap this precision type
#endif

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
    int constraints = -1;
    int poles = -1;
    int target_rotations = -1;
    int integrate_rk45 = -1;
    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* alg = (struct ik_algorithm*)self->super.attachment;

    static char* kwds_names[] = {
        "type",
        "max_iterations",
        "tolerance",
        "constraints",
        "poles",
        "target_rotations",
        "integrate_rk45",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|H" FMT "pppp", kwds_names,
                                     &name,
                                     &alg->max_iterations,
                                     &alg->tolerance,
                                     &constraints,
                                     &poles,
                                     &target_rotations,
                                     &integrate_rk45))
    {
        return -1;
    }

    if (ik_algorithm_set_type(alg, name) != 0)
    {
        PyErr_Format(PyExc_ValueError, "Invalid algorithm name was specified: `%s`");
        return -1;
    }

    if (constraints == 1)
        alg->features |= IK_ALGORITHM_CONSTRAINTS;
    else if (constraints == 0)
        alg->features &= ~IK_ALGORITHM_CONSTRAINTS;

    if (poles == 1)
        alg->features |= IK_ALGORITHM_POLES;
    else if (poles == 0)
        alg->features &= ~IK_ALGORITHM_POLES;

    if (target_rotations == 1)
        alg->features |= IK_ALGORITHM_TARGET_ROTATIONS;
    else if (target_rotations == 0)
        alg->features &= ~IK_ALGORITHM_TARGET_ROTATIONS;

    if (integrate_rk45 == 1)
        alg->features |= IK_ALGORITHM_INTEGRATE_RK45;
    else if (integrate_rk45 == 0)
        alg->features &= ~IK_ALGORITHM_INTEGRATE_RK45;

    return 0;
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
static PyObject*
Algorithm_repr_build_arglist_list(PyObject* myself)
{
    ik_Algorithm* self = (ik_Algorithm*)myself;
    struct ik_algorithm* alg = (struct ik_algorithm*)self->super.attachment;

    PyObject* args = PyList_New(0);
    if (args == NULL)
        return NULL;

    /* Type */
    {
        int append_result;
        PyObject* arg = PyUnicode_FromFormat("\"%s\"", alg->type);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

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
PyDoc_STRVAR(ALGORITHM_DOC, "");
PyTypeObject ik_AlgorithmType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Algorithm",
    .tp_basicsize = sizeof(ik_Algorithm),
    .tp_dealloc = Algorithm_dealloc,
    .tp_repr = Algorithm_repr,
    .tp_str = Algorithm_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = ALGORITHM_DOC,
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
