#include "Python.h"
#include "ik/init.h"
#include "ik/algorithm.h"
#include "ik/log.h"
#include "ik/python/ik_docstrings.h"
#include "ik/python/ik_module.h"
#include "ik/python/ik_type_Log.h"
#include "ik/python/ik_type_Algorithm.h"
#include "ik/python/ik_type_Bone.h"
#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_Info.h"
#include "ik/python/ik_type_Mat3x4.h"
#include "ik/python/ik_type_TreeObject.h"
#include "ik/python/ik_type_Pole.h"
#include "ik/python/ik_type_Pose.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Vec3.h"

#if defined(IK_LOGGING)
#   define LOG_TYPE X(Log)
#else
#   define LOG_TYPE
#endif

#define PUBLIC_IK_TYPES  \
    X(Attachment)        \
    X(Algorithm)         \
    X(Bone)              \
    X(Constraint)        \
    X(HingeConstraint)   \
    X(StiffConstraint)   \
    X(Effector)          \
    X(Mat3x4)            \
    X(Pose)              \
    X(Pole)              \
    X(BlenderPole)       \
    X(GenericPole)       \
    X(MayaPole)          \
    X(Quat)              \
    X(Solver)            \
    X(TreeObject)        \
    X(Vec3)

#define PRIVATE_IK_TYPES \
    X(Info)              \
    LOG_TYPE

/* ------------------------------------------------------------------------- */
static void
module_incref(void)
{
    PyObject* module = PyState_FindModule(&ik_module);
    Py_INCREF(module);
}

/* ------------------------------------------------------------------------- */
static void
module_decref(void)
{
    PyObject* module = PyState_FindModule(&ik_module);
    Py_DECREF(module);
}

/* ------------------------------------------------------------------------- */
static int g_refs = 0;
static void
public_type_instantiated_callback(PyObject* o)
{
    module_incref();
    g_refs++;
    printf("incref (%d)\n", g_refs);
}

/* ------------------------------------------------------------------------- */
static void
public_type_deleted_callback(PyObject* o)
{
    module_decref();
    g_refs--;
    printf("decref (%d)\n", g_refs);
}

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
#define X(typename)                                                           \
        static newfunc typename##_new_orig;                                   \
        static destructor typename##_dealloc_orig;                            \
        static PyObject* typename##_new_wrapper(PyTypeObject* type, PyObject* args, PyObject* kwds) \
        {                                                                     \
            PyObject* o = (*typename##_new_orig)(type, args, kwds);           \
            if (o)                                                            \
                public_type_instantiated_callback(o);                         \
            return o;                                                         \
        }                                                                     \
        static void typename##_dealloc_wrapper(PyObject* o)                   \
        {                                                                     \
            public_type_deleted_callback(o);                                  \
            (*typename##_dealloc_orig)(o);                                    \
        }                                                                     \
        static void wrap_##typename##_type(void)                              \
        {                                                                     \
            /* module init func can be called again, guard against that */    \
            if (typename##_new_orig == NULL)                                  \
            {                                                                 \
                typename##_new_orig = ik_##typename##Type.tp_new;             \
                typename##_dealloc_orig = ik_##typename##Type.tp_dealloc;     \
                                                                              \
                ik_##typename##Type.tp_new = typename##_new_wrapper;          \
                ik_##typename##Type.tp_dealloc = typename##_dealloc_wrapper;  \
            }                                                                 \
        }
    PUBLIC_IK_TYPES
#undef X

/* ------------------------------------------------------------------------- */
static int
init_builtin_types(PyObject* m)
{
    /* Init all IK types */
#define X(typename)                          \
        if (init_ik_##typename##Type() != 0) \
            return -1;
    PUBLIC_IK_TYPES
    PRIVATE_IK_TYPES
#undef X

    /* Insert callbacks into all public IK type objects so we know when they get
     * allocated and deleted. This is important because we must ensure that the
     * module object outlives any types instantiated from the module. */
#define X(typename) wrap_##typename##_type();
    PUBLIC_IK_TYPES
#undef X

    /* Add all public IK types to the module so they can be instantiated in
     * python */
#define X(typename)                                                           \
        Py_INCREF(&ik_##typename##Type);                                      \
        if (PyModule_AddObject(m, #typename, (PyObject*)&ik_##typename##Type) != 0) \
        {                                                                     \
            Py_DECREF(&ik_##typename##Type);                                  \
            return -1;                                                        \
        }
    PUBLIC_IK_TYPES
#undef X
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
add_type_to_module(PyObject* m, const char* name, PyTypeObject* type)
{
    PyObject* o = PyObject_CallObject((PyObject*)type, NULL);
    if (o == NULL)
        return -1;

    if (PyModule_AddObject(m, name, o) == 0)
        return 0;

    Py_DECREF(o);
    return -1;
}
static int
add_builtin_objects_to_module(PyObject* m)
{
    if (add_type_to_module(m, "info", &ik_InfoType) != 0) return -1;
#if defined(IK_LOGGING)
    if (add_type_to_module(m, "info", &ik_LogType) != 0)  return -1;
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

    if (init_builtin_types(m) != 0)            goto init_module_failed;
    if (add_builtin_objects_to_module(m) != 0) goto init_module_failed;
    if (add_constants_to_module(m) != 0)       goto init_module_failed;

    return m;

    init_module_failed  : Py_DECREF(m);
    return NULL;
}
