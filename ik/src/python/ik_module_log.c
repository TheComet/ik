#include "Python.h"

#include "ik/python/ik_module_log.h"
#include "ik/ik.h"

/* ------------------------------------------------------------------------- */
static PyObject*
log_message(PyObject* self, PyObject* args)
{
    (void)self;
    PyObject* uni;
    PyObject* ascii;

    /* Convert to string, might be necessary */
    if ((uni = PyObject_Str(args)) == NULL)
        goto str_call_failed;
    if ((ascii = PyUnicode_AsASCIIString(uni)) == NULL)
        goto ascii_conversion_failed;

    IKAPI.log.message("%s", PyBytes_AS_STRING(ascii));

    Py_DECREF(ascii);
    Py_DECREF(uni);
    Py_RETURN_NONE;

    ascii_conversion_failed : Py_DECREF(uni);
    str_call_failed         : return NULL;
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
    {"message", log_message, METH_O, "Log a message to the library."},
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
