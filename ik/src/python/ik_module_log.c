#include "Python.h"

#include "ik/python/ik_module_log.h"
#include "ik/ik.h"

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
static void
module_free(void* x)
{
    (void)x;
    IKAPI.log.deinit();
}

/* ------------------------------------------------------------------------- */
static PyMethodDef log_functions[] = {
    {"debug",   log_debug,   METH_O, "Log a debug message to the library."},
    {"info",    log_info,    METH_O, "Log an info message to the library."},
    {"warning", log_warning, METH_O, "Log a warning message to the library."},
    {"error",   log_error,   METH_O, "Log an error message to the library."},
    {"fatal",   log_fatal,   METH_O, "Log a fatal message to the library."},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyModuleDef ik_module_log = {
    PyModuleDef_HEAD_INIT,
    "log",                   /* Module name */
    NULL,                    /* docstring, may be NULL */
    -1,                      /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables */
    log_functions,           /* module methods */
    NULL,                    /* m_reload */
    NULL,                    /* m_traverse */
    NULL,                    /* m_clear */
    module_free              /* m_free */
};

/* ------------------------------------------------------------------------- */
PyObject*
ik_module_log_create(void)
{
    PyObject* m;

    if (IKAPI.log.init() != IK_OK)
        goto ik_log_init_failed;

    m = PyModule_Create(&ik_module_log);
    if (m == NULL)
        goto module_alloc_failed;

    return m;

    module_alloc_failed : IKAPI.log.deinit();
    ik_log_init_failed  : return NULL;
}
