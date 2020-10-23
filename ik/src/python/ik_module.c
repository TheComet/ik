#include "Python.h"
#include "ik/init.h"
#include "ik/algorithm.h"
#include "ik/log.h"
#include "ik/python/ik_docstrings.h"
#include "ik/python/ik_module.h"
#include "ik/python/ik_type_Log.h"
#include "ik/python/ik_type_Algorithm.h"
#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_Info.h"
#include "ik/python/ik_type_Mat3x4.h"
#include "ik/python/ik_type_ModuleRef.h"
#include "ik/python/ik_type_TreeObject.h"
#include "ik/python/ik_type_Pole.h"
#include "ik/python/ik_type_Pose.h"
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
PyModuleDef ik_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "ik",
    .m_doc = IK_MODULE_DOC,
    .m_size = -1,
    .m_free = module_free
};

/* ------------------------------------------------------------------------- */
static int
init_builtin_types(void)
{
    if (init_ik_AttachmentType() != 0)     return -1;
    if (init_ik_AlgorithmType() != 0)      return -1;
    if (init_ik_ConstraintType() != 0)     return -1;
    if (init_ik_EffectorType() != 0)       return -1;
    if (init_ik_InfoType() != 0)           return -1;
#if defined(IK_LOGGING)
    if (init_ik_LogType() != 0)            return -1;
#endif
    if (init_ik_Mat3x4Type() != 0)         return -1;
    if (init_ik_ModuleRefType() != 0)      return -1;
    if (init_ik_PoleType() != 0)           return -1;
    if (init_ik_PoseType() != 0)           return -1;
    if (init_ik_QuatType() != 0)           return -1;
    if (init_ik_SolverType() != 0)         return -1;
    if (init_ik_TreeObjectType() != 0)     return -1;
    if (init_ik_Vec3Type() != 0)           return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_types_to_module(PyObject* m)
{
#define ADD_TYPE(name) do {                                                   \
        Py_INCREF(&ik_##name##Type);                                          \
        if (PyModule_AddObject(m, #name, (PyObject*)&ik_##name##Type) != 0)   \
        {                                                                     \
            Py_DECREF(&ik_##name##Type);                                      \
            return -1;                                                        \
        }                                                                     \
    } while(0)

    ADD_TYPE(Attachment);
    ADD_TYPE(Algorithm);
    ADD_TYPE(BlenderPole);
    ADD_TYPE(Constraint);
    ADD_TYPE(Effector);
    ADD_TYPE(GenericPole);
    ADD_TYPE(HingeConstraint);
    ADD_TYPE(Mat3x4);
    ADD_TYPE(MayaPole);
    ADD_TYPE(Pose);
    ADD_TYPE(Pole);
    ADD_TYPE(Solver);
    ADD_TYPE(StiffConstraint);
    ADD_TYPE(TreeObject);
    ADD_TYPE(Quat);
    ADD_TYPE(Vec3);

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

    /* Log constants */
#if defined(IK_LOGGING)
#   define X(arg) if (PyModule_AddIntConstant(m, #arg, IK_##arg) != 0) return -1;
    IK_LOG_SEVERITY_LIST
#   undef X
#endif

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_objects_to_module(PyObject* m)
{
    {
        PyObject* info = PyObject_CallObject((PyObject*)&ik_InfoType, NULL);
        if (info == NULL)
            return -1;
        if (PyModule_AddObject(m, "info", info) != 0)
        {
            Py_DECREF(info);
            return -1;
        }
    }

    /* Instantiate log and add to module */
#if defined(IK_LOGGING)
    {
        PyObject* log = PyObject_CallObject((PyObject*)&ik_LogType, NULL);
        if (log == NULL)
            return -1;
        if (PyModule_AddObject(m, "log", log) != 0)
        {
            Py_DECREF(log);
            return -1;
        }
    }
#endif

    return 0;
}

/* ------------------------------------------------------------------------- */
PyMODINIT_FUNC PyInit_ik(void)
{
    PyObject* m;

    if (ik_init() < 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to initialize ik library");
        return NULL;
    }

    m = PyModule_Create(&ik_module);
    if (m == NULL)
    {
        ik_deinit();
        return NULL;
    }

    if (init_builtin_types() != 0)             goto init_module_failed;
    if (add_builtin_types_to_module(m) != 0)   goto init_module_failed;
    if (add_builtin_objects_to_module(m) != 0) goto init_module_failed;
    if (add_constants_to_module(m) != 0)       goto init_module_failed;

    return m;

    init_module_failed  : Py_DECREF(m);
    return NULL;
}
