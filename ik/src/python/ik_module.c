#include "Python.h"
#include "ik/ik.h"
#include "ik/python/ik_module_info.h"
#include "ik/python/ik_module_log.h"
#include "ik/python/ik_module_node.h"
#include "ik/python/ik_module_quat.h"
#include "ik/python/ik_module_solver.h"
#include "ik/python/ik_module_vec3.h"

/* ------------------------------------------------------------------------- */
static void
module_free(void* x)
{
    (void)x;
    ik.deinit();
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
    if (init_ik_NodeType() != 0)   return -1;
    if (init_ik_QuatType() != 0)   return -1;
    if (init_ik_SolverType() != 0) return -1;
    if (init_ik_Vec3Type() != 0)   return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_types_to_module(PyObject* m)
{
    Py_INCREF(&ik_NodeType);   if (PyModule_AddObject(m, "Node",   (PyObject*)&ik_NodeType) < 0)   return -1;
    Py_INCREF(&ik_QuatType);   if (PyModule_AddObject(m, "Quat",   (PyObject*)&ik_QuatType) < 0)   return -1;
    Py_INCREF(&ik_SolverType); if (PyModule_AddObject(m, "Solver", (PyObject*)&ik_SolverType) < 0) return -1;
    Py_INCREF(&ik_Vec3Type);   if (PyModule_AddObject(m, "Vec3",   (PyObject*)&ik_Vec3Type) < 0)   return -1;
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

    if (ik.init() != IK_OK)
        goto ik_init_failed;

    m = PyModule_Create(&ik_module);
    if (m == NULL)
        goto module_alloc_failed;

    if (init_builtin_types() != 0)            goto init_module_failed;
    if (add_builtin_types_to_module(m) != 0)  goto init_module_failed;
    if (add_submodules_to_module(m) != 0)     goto init_module_failed;

    return m;

    init_module_failed            : Py_DECREF(m);
    module_alloc_failed           : ik.deinit();
    ik_init_failed                : return NULL;
}
