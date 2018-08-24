#include "Python.h"
#include "ik/python/ik_module_log.h"
#include "ik/ik.h"

typedef struct ik_Log
{
    PyObject_HEAD
} ik_Log;

/* ------------------------------------------------------------------------- */
static void
Log_dealloc(ik_Log* self)
{
    (void)self;
    IKAPI.log.deinit();
}

/* ------------------------------------------------------------------------- */
static PyObject*
Log_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    (void)args;
    (void)kwds;
    ik_Log* self;

    if (IKAPI.log.init() != IK_OK)
        goto ik_log_init_failed;

    self = (ik_Log*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : IKAPI.log.deinit();
    ik_log_init_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
log_message(PyObject* args, void (*log_func)(const char* fmt, ...))
{
    PyObject* uni;
    PyObject* ascii;

    /* Convert to string, might be necessary */
    if ((uni = PyObject_Str(args)) == NULL)
        goto str_call_failed;
    if ((ascii = PyUnicode_AsASCIIString(uni)) == NULL)
        goto ascii_conversion_failed;

    log_func("%s", PyBytes_AS_STRING(ascii));

    Py_DECREF(ascii);
    Py_DECREF(uni);
    Py_RETURN_NONE;

    ascii_conversion_failed : Py_DECREF(uni);
    str_call_failed         : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
log_debug(PyObject* self, PyObject* args)
{
    (void)self;
    return log_message(args, IKAPI.log.debug);
}
static PyObject*
log_info(PyObject* self, PyObject* args)
{
    (void)self;
    return log_message(args, IKAPI.log.info);
}
static PyObject*
log_warning(PyObject* self, PyObject* args)
{
    (void)self;
    return log_message(args, IKAPI.log.warning);
}
static PyObject*
log_error(PyObject* self, PyObject* args)
{
    (void)self;
    return log_message(args, IKAPI.log.error);
}
static PyObject*
log_fatal(PyObject* self, PyObject* args)
{
    (void)self;
    return log_message(args, IKAPI.log.fatal);
}

/* ------------------------------------------------------------------------- */
static int
Log_setseverity(ik_Log* self, PyObject* value, void* closure)
{
    (void)self;
    (void)closure;
    int severity;

    if (!PyLong_Check(value))
    {
        PyErr_SetString(PyExc_ValueError, "Expected a value. Use ik.DEVEL, ik.INFO, ik.WARNING, ik.ERROR or ik.FATAL");
        return -1;
    }

    severity = PyLong_AS_LONG(value);
    if (severity < IK_LOG_DEVEL || severity > IK_LOG_FATAL)
    {
        PyErr_SetString(PyExc_ValueError, "Value out of range. Expected eitehr ik.DEVEL, ik.INFO, ik.WARNING, ik.ERROR or ik.FATAL");
        return -1;
    }

    IKAPI.log.severity(severity);

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
Log_settimestamps(ik_Log* self, PyObject* value, void* closure)
{
    (void)self;
    (void)closure;

    if (!PyBool_Check(value))
    {
        PyErr_SetString(PyExc_ValueError, "Expected true or false");
        return -1;
    }

    IKAPI.log.timestamps(PyObject_IsTrue(value));

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
Log_setprefix(ik_Log* self, PyObject* value, void* closure)
{
    (void)self;
    (void)closure;

    if (PyUnicode_Check(value))
    {
        PyObject* ascii = PyUnicode_AsASCIIString(value);
        if (ascii == NULL)
            return -1;

        IKAPI.log.prefix(PyBytes_AS_STRING(ascii));
        Py_DECREF(ascii);
    }
    else if(value == Py_None)
    {
        IKAPI.log.prefix("");
    }
    else
    {
        PyErr_SetString(PyExc_ValueError, "Expected a string or None");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Log_methods[] = {
    {"debug",   log_debug,   METH_O, "Log a debug message to the library."},
    {"info",    log_info,    METH_O, "Log an info message to the library."},
    {"warning", log_warning, METH_O, "Log a warning message to the library."},
    {"error",   log_error,   METH_O, "Log an error message to the library."},
    {"fatal",   log_fatal,   METH_O, "Log a fatal message to the library."},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyGetSetDef Log_getsetters[] = {
    {"severity",   NULL, (setter)Log_setseverity,   "Sets the log severity level", NULL},
    {"timestamps", NULL, (setter)Log_settimestamps, "Enables or disables timestamps", NULL},
    {"prefix",     NULL, (setter)Log_setprefix,     "Sets the log's prefix", NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyTypeObject ik_LogType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ik.Log",                                      /* tp_name */
    sizeof(ik_Log),                                /* tp_basicsize */
    0,                                             /* tp_itemsize */
    (destructor)Log_dealloc,                       /* tp_dealloc */
    0,                                             /* tp_print */
    0,                                             /* tp_getattr */
    0,                                             /* tp_setattr */
    0,                                             /* tp_reserved */
    0,                                             /* tp_repr */
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
    Log_methods,                                   /* tp_methods */
    0,                                             /* tp_members */
    Log_getsetters,                                /* tp_getset */
    0,                                             /* tp_base */
    0,                                             /* tp_dict */
    0,                                             /* tp_descr_get */
    0,                                             /* tp_descr_set */
    0,                                             /* tp_dictoffset */
    0,                                             /* tp_init */
    0,                                             /* tp_alloc */
    Log_new                                        /* tp_new */
};

/* ------------------------------------------------------------------------- */
PyObject*
ik_module_log_create(void)
{
    PyObject* m;

    if (PyType_Ready(&ik_LogType) < 0)
        return NULL;

    m = PyObject_CallObject((PyObject*)&ik_LogType, NULL);
    if (m == NULL)
        return NULL;

    return m;
}
