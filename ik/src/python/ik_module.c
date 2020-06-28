#include "Python.h"
#include "ik/init.h"
#include "ik/algorithm.h"
#include "ik/python/ik_module.h"
#include "ik/python/ik_module_log.h"
#include "ik/python/ik_type_Algorithm.h"
#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_Info.h"
#include "ik/python/ik_type_ModuleRef.h"
#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Vec3.h"

/* ------------------------------------------------------------------------- */
static void
module_free(void* x)
{
    (void)x;
    ik_deinit();
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(IK_MODULE_DOC,
"");
PyModuleDef ik_module = {
    PyModuleDef_HEAD_INIT,
    "ik",                    /* Module name */
    IK_MODULE_DOC,           /* docstring, may be NULL */
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
    if (init_ik_AttachmentType() != 0) return -1;
    if (init_ik_AlgorithmType() != 0)  return -1;
    if (init_ik_EffectorType() != 0)   return -1;
    if (init_ik_InfoType() != 0)       return -1;
    if (init_ik_ModuleRefType() != 0)  return -1;
    if (init_ik_NodeType() != 0)       return -1;
    if (init_ik_QuatType() != 0)       return -1;
    if (init_ik_SolverType() != 0)     return -1;
    if (init_ik_Vec3Type() != 0)       return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_types_to_module(PyObject* m)
{
    Py_INCREF(&ik_AttachmentType); if (PyModule_AddObject(m, "Attachment", (PyObject*)&ik_AttachmentType) != 0) return -1;
    Py_INCREF(&ik_AlgorithmType);  if (PyModule_AddObject(m, "Algorithm",  (PyObject*)&ik_AlgorithmType) != 0)  return -1;
    Py_INCREF(&ik_EffectorType);   if (PyModule_AddObject(m, "Effector",   (PyObject*)&ik_EffectorType) != 0)   return -1;
    Py_INCREF(&ik_NodeType);       if (PyModule_AddObject(m, "Node",       (PyObject*)&ik_NodeType) != 0)       return -1;
    Py_INCREF(&ik_QuatType);       if (PyModule_AddObject(m, "Quat",       (PyObject*)&ik_QuatType) != 0)       return -1;
    Py_INCREF(&ik_SolverType);     if (PyModule_AddObject(m, "Solver",     (PyObject*)&ik_SolverType) != 0)     return -1;
    Py_INCREF(&ik_Vec3Type);       if (PyModule_AddObject(m, "Vec3",       (PyObject*)&ik_Vec3Type) != 0)       return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_constants_to_module(PyObject* m)
{
    /* Built in algorithm names */
#define X(algo) if (PyModule_AddStringConstant(m, #algo, IK_##algo) == -1) return -1;
    IK_ALGORITHM_LIST
#undef X

    /* Log constants *
#define X(arg) if (PyModule_AddIntConstant(m, #arg, IK_LOG_##arg) != 0) return -1;
    IK_LOG_SEVERITY_LIST
#undef X*/

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_objects_to_module(PyObject* m)
{
    PyObject* o = PyObject_CallObject((PyObject*)&ik_InfoType, NULL);
    if (o == NULL)
        return -1;
    if (PyModule_AddObject(m, "info", o) != 0)
        return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */
PyMODINIT_FUNC PyInit_ik(void)
{
    PyObject* m;

    if (ik_init() < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to initialize ik library");
        goto ik_init_failed;
    }

    m = PyModule_Create(&ik_module);
    if (m == NULL)
        goto module_alloc_failed;

    if (init_builtin_types() != 0)             goto init_module_failed;
    if (add_builtin_types_to_module(m) != 0)   goto init_module_failed;
    if (add_builtin_objects_to_module(m) != 0) goto init_module_failed;
    if (add_constants_to_module(m) != 0)       goto init_module_failed;

    return m;

    init_module_failed  : Py_DECREF(m);
    module_alloc_failed : ik_deinit();
    ik_init_failed      : return NULL;
}
