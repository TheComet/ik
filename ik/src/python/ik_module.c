#include "Python.h"
#include "ik/ik.h"
#include "ik/python/init.h"
#include "ik/python/ik_module_info.h"
#include "ik/python/ik_module_log.h"

/* ------------------------------------------------------------------------- */
static void
module_free(void* x)
{
    (void)x;
    deinit_iklib_refcounted();
}

/* ------------------------------------------------------------------------- */
static PyModuleDef ik_module = {
    PyModuleDef_HEAD_INIT,
    "ik",                    /* Module name */
    NULL,                    /* docstring, may be NULL */
    -1,                      /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables */
    NULL,                    /* module methods */
    NULL,                    /* m_reload */
    NULL,                    /* m_traverse */
    NULL,                    /* m_clear */
    module_free              /* m_free */
};

/* ------------------------------------------------------------------------- */
static int
init_builtin_types(void)
{
    /*if (ik_init_solver_FABRIK() != 0)     return -1;*/
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_types_to_module(PyObject* m)
{
    (void)m;
    /*Py_INCREF(&ik_Log);

    if (PyModule_AddObject(m, "log",          (PyObject*)&ik_AttributeType) < 0)    return -1;
    if (PyModule_AddObject(m, "Face",         (PyObject*)&ik_FaceType) < 0)         return -1;
    if (PyModule_AddObject(m, "Mesh",         (PyObject*)&ik_MeshType) < 0)         return -1;
    if (PyModule_AddObject(m, "MeshIterator", (PyObject*)&ik_MeshIteratorType) < 0) return -1;
    if (PyModule_AddObject(m, "Vertex",       (PyObject*)&ik_VertexType) < 0)       return -1;*/
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_submodules_to_module(PyObject* m)
{
    PyObject* submodule;

    submodule = ik_module_info_create();
    if (submodule == NULL)
        return -1;
    if (PyModule_AddObject(m, "info", submodule) < 0)
    {
        Py_DECREF(submodule);
        return -1;
    }

    submodule = ik_module_log_create();
    if (submodule == NULL)
        return -1;
    if (PyModule_AddObject(m, "log", submodule) < 0)
    {
        Py_DECREF(submodule);
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
PyMODINIT_FUNC
PyInit_ik(void)
{
    PyObject* m;

    if (init_iklib_refcounted() != 0)
        goto ik_init_failed;

    m = PyModule_Create(&ik_module);
    if (m == NULL)
        goto module_alloc_failed;

    if (init_builtin_types() != 0)            goto init_module_failed;
    if (add_builtin_types_to_module(m) != 0)  goto init_module_failed;
    if (add_submodules_to_module(m) != 0)     goto init_module_failed;

    return m;

    init_module_failed            : Py_DECREF(m);
    module_alloc_failed           : deinit_iklib_refcounted();
    ik_init_failed                : return NULL;
}
