#include "Python.h"
#include "ik/python/ik_type_Log.h"
#include "ik/python/ik_docstrings.h"
#include "ik/log.h"

/* ------------------------------------------------------------------------- */
static const char* severities[] = {
#define X(s) #s,
    IK_LOG_SEVERITY_LIST
#undef X
};
static void
log_callback(void* myself, enum ik_log_severity severity, const char* msg)
{
    ik_Log* self = (ik_Log*)myself;

    if (self->on_message != Py_None)
    {
        PyObject *args, *py_severity, *py_msg, *result;

        py_severity = PyLong_FromLong(severity);
        if (py_severity == NULL)
            goto py_severity_failed;

        py_msg = PyUnicode_FromString(msg);
        if (py_msg == NULL)
            goto py_msg_failed;

        args = PyTuple_New(2);
        if (args == NULL)
            goto args_failed;

        PyTuple_SET_ITEM(args, 0, py_severity);
        PyTuple_SET_ITEM(args, 1, py_msg);
        result = PyObject_CallObject(self->on_message, args);
        Py_DECREF(args);

        Py_XDECREF(result);
        return;

        args_failed        : Py_DECREF(py_msg);
        py_msg_failed      : Py_DECREF(py_severity);
        py_severity_failed : return;
    }

    if ((int)severity < self->severity)
        return;

    if (self->timestamps)
    {
        char timestamp[9];
        time_t rawtime = time(NULL); /* get system time */
        struct tm* timeinfo = localtime(&rawtime); /* convert to local time */
        strftime(timestamp, 9, "%H:%M:%S", timeinfo);  /* HH:MM:SS + null = 9 bytes */
        fprintf(stderr, "[%s] ", timestamp);
    }

    fprintf(stderr, "IK %s: %s\n", severities[severity], msg);
}

/* ------------------------------------------------------------------------- */
static void
Log_dealloc(PyObject* myself)
{
    ik_Log* self = (ik_Log*)myself;
    Py_DECREF(self->on_message);
    ik_log_deinit();
    Py_TYPE(myself)->tp_free(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Log_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Log* self;
    (void)args;
    (void)kwds;

    if (ik_log_init() < 0)
        goto ik_log_init_failed;

    self = (ik_Log*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    self->timestamps = 1;
    self->severity = IK_INFO;
    self->on_message = Py_None;
    Py_INCREF(Py_None);

    ik_log_set_callback(log_callback, (void*)self);

    return (PyObject*)self;

    alloc_self_failed  : ik_log_deinit();
    ik_log_init_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
log_message(PyObject* args, enum ik_log_severity severity)
{
    PyObject* uni;
    PyObject* ascii;

    /* Convert to string, might be necessary */
    if ((uni = PyObject_Str(args)) == NULL)
        goto str_call_failed;
    if ((ascii = PyUnicode_AsASCIIString(uni)) == NULL)
        goto ascii_conversion_failed;

    ik_log_printf(severity, "%s", PyBytes_AS_STRING(ascii));

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
    return log_message(args, IK_DEBUG);
}
static PyObject*
log_info(PyObject* self, PyObject* args)
{
    (void)self;
    return log_message(args, IK_INFO);
}
static PyObject*
log_warning(PyObject* self, PyObject* args)
{
    (void)self;
    return log_message(args, IK_WARN);
}
static PyObject*
log_error(PyObject* self, PyObject* args)
{
    (void)self;
    return log_message(args, IK_ERROR);
}
static PyObject*
log_fatal(PyObject* self, PyObject* args)
{
    (void)self;
    return log_message(args, IK_FATAL);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Log_getseverity(PyObject* myself, void* closure)
{
    ik_Log* self = (ik_Log*)myself;
    (void)closure;

    return PyLong_FromLong(self->severity);
}
static int
Log_setseverity(PyObject* myself, PyObject* value, void* closure)
{
    int severity;
    ik_Log* self = (ik_Log*)myself;
    (void)closure;

    if (!PyLong_Check(value))
    {
        PyErr_SetString(PyExc_ValueError, "Expected a value. Use one of "
#define X(name) "ik." #name "  "
        IK_LOG_SEVERITY_LIST
#undef X
        );
        return -1;
    }

    severity = PyLong_AS_LONG(value);
    if (severity < IK_DEBUG || severity > IK_FATAL)
    {
        PyErr_SetString(PyExc_ValueError, "Value out of range. Expected one of "
#define X(name) "ik." #name "  "
        IK_LOG_SEVERITY_LIST
#undef X
        );
        return -1;
    }

    self->severity = severity;

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Log_gettimestamps(PyObject* myself, void* closure)
{
    ik_Log* self = (ik_Log*)myself;
    (void)closure;

    return PyBool_FromLong(self->timestamps);
}
static int
Log_settimestamps(PyObject* myself, PyObject* value, void* closure)
{
    ik_Log* self = (ik_Log*)myself;
    (void)closure;

    if (!PyBool_Check(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected true or false");
        return -1;
    }

    self->timestamps = PyObject_IsTrue(value);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Log_geton_message(PyObject* myself, void* closure)
{
    ik_Log* self = (ik_Log*)myself;
    (void)closure;

    return Py_INCREF(self->on_message), self->on_message;
}
static int
Log_seton_message(PyObject* myself, PyObject* value, void* closure)
{
    PyObject* tmp;
    ik_Log* self = (ik_Log*)myself;
    (void)closure;

    if (value != Py_None && !PyCallable_Check(value))
    {
        PyErr_SetString(PyExc_TypeError, "Object is not callable");
        return -1;
    }

    tmp = self->on_message;
    Py_INCREF(value);
    self->on_message = value;
    Py_DECREF(tmp);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Log_methods[] = {
    {"debug",   log_debug,   METH_O, IK_LOG_DEBUG_DOC},
    {"info",    log_info,    METH_O, IK_LOG_INFO_DOC},
    {"warning", log_warning, METH_O, IK_LOG_WARNING_DOC},
    {"error",   log_error,   METH_O, IK_LOG_ERROR_DOC},
    {"fatal",   log_fatal,   METH_O, IK_LOG_FATAL_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyGetSetDef Log_getsetters[] = {
    {"severity",   Log_getseverity,   Log_setseverity,   IK_LOG_SEVERITY_DOC, NULL},
    {"timestamps", Log_gettimestamps, Log_settimestamps, IK_LOG_TIMESTAMPS_DOC, NULL},
    {"on_message", Log_geton_message, Log_seton_message, IK_LOG_ON_MESSAGE_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject ik_LogType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Log",
    .tp_basicsize = sizeof(ik_Log),
    .tp_dealloc = Log_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_LOG_DOC,
    .tp_methods = Log_methods,
    .tp_getset = Log_getsetters,
    .tp_new = Log_new
};

/* ------------------------------------------------------------------------- */
int
init_ik_LogType(void)
{
    if (PyType_Ready(&ik_LogType) < 0)
        return -1;

    return 0;
}
