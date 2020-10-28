#include "Python.h"
#include "cstructures/hashmap.h"
#include "cstructures/hash.h"
#include "cstructures/backtrace.h"
#include "ik/init.h"
#include "ik/algorithm.h"
#include "ik/log.h"
#include "ik/python.h"
#include "ik/python/ik_docstrings.h"
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
#include <string.h>

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

#if defined(IK_PYTHON_REFCOUNT_DEBUGGING)
struct module_state
{
    int active_instance_count;
    struct cs_hashmap active_instances;
};
#endif

/* ------------------------------------------------------------------------- */
static void
module_free(void* m)
{
#if defined(IK_PYTHON_REFCOUNT_DEBUGGING)
    struct module_state* state = PyModule_GetState(m);
    hashmap_deinit(&state->active_instances);
#else
    (void)m;
#endif

    ik_deinit();
}

/* ------------------------------------------------------------------------- */
#if defined(IK_PYTHON_REFCOUNT_DEBUGGING)
static PyObject* ik_module_active_instances(PyObject* m, PyObject* args);
static PyObject* ik_module_print_active_instances(PyObject* m, PyObject* args);
#if defined(IK_PYTHON_REFCOUNT_BACKTRACES)
static PyObject* ik_module_print_active_instance_backtraces(PyObject* m, PyObject* args);
#endif
#endif

static PyMethodDef ik_methods[] = {
#if defined(IK_PYTHON_REFCOUNT_DEBUGGING)
    {"active_instances", ik_module_active_instances, METH_NOARGS, ""},
    {"print_active_instances", ik_module_print_active_instances, METH_NOARGS, ""},
#if defined(IK_PYTHON_REFCOUNT_BACKTRACES)
    {"print_active_instance_backtraces", ik_module_print_active_instance_backtraces, METH_NOARGS, ""},
#endif
#endif
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyModuleDef ik_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "ik",
    .m_doc = IK_MODULE_DOC,
#if defined(IK_PYTHON_REFCOUNT_DEBUGGING)
    .m_size = sizeof(struct module_state),
#else
    .m_size = -1,
#endif
    .m_methods = ik_methods,
    .m_free = module_free
};

/* ------------------------------------------------------------------------- */
#if defined(IK_PYTHON_REFCOUNT_DEBUGGING)

struct active_instance
{
    PyObject* obj;
#if defined(IK_PYTHON_REFCOUNT_BACKTRACES)
    int backtrace_size;
    char** backtrace;
#endif
};

struct instance_key
{
    PyObject* obj;
    char typename[8];
};

static void
strncpy_no_null(char* dst, const char* src, int len)
{
    while (len-- && *src != '\0')
        *dst++ = *src++;
    while (len-- > 0)
        *dst++ = '\0';
}

static void
debug_refs_inc(PyObject* m, PyObject* o, const char* typename)
{
    struct active_instance inst;
    struct instance_key key;
    struct module_state* state = PyModule_GetState(m);
    state->active_instance_count++;

    inst.obj = o;
#if defined(IK_PYTHON_REFCOUNT_BACKTRACES)
    inst.backtrace = get_backtrace(&inst.backtrace_size);
    if (inst.backtrace == NULL)
        fprintf(stderr, "[debugrefs] WARNING: Failed to generate backtrace\n");
#endif

    key.obj = o;
    strncpy_no_null(key.typename, typename, 8);
    if (hashmap_insert(&state->active_instances, &key, &inst) != HM_OK)
        fprintf(stderr, "[debugrefs] WARNING: Hashmap insert failed\n");
}
static void
debug_refs_dec(PyObject* m, PyObject* o, const char* typename)
{
    struct active_instance* inst;
    struct instance_key key;
    struct module_state* state = PyModule_GetState(m);
    state->active_instance_count--;

    key.obj = o;
    strncpy_no_null(key.typename, typename, 8);
    inst = hashmap_erase(&state->active_instances, &key);
    if (inst)
    {
#if defined(IK_PYTHON_REFCOUNT_BACKTRACES)
        if (inst->backtrace)
            free(inst->backtrace);
        else
            fprintf(stderr, "[debugrefs] WARNING: No backtrace exists for object being deallocated\n");
#endif
    }
    else
    {
        fprintf(stderr, "[debugrefs] WARNING: Deallocating an object that was never allocated\n");
    }
}
int
ik_python_active_instances(void)
{
    struct module_state* state;
    PyObject* m = PyState_FindModule(&ik_module);
    if (m == NULL)
        return 0;

    state = PyModule_GetState(m);
    return state->active_instance_count;
}
void
ik_python_print_active_instances(void)
{
    struct module_state* state;
    PyObject* m = PyState_FindModule(&ik_module);
    if (m == NULL)
        return;
    state = PyModule_GetState(m);

    fprintf(stderr, "=========================================\n");
    HASHMAP_FOR_EACH(&state->active_instances, PyObject*, struct active_instance, pobj, inst)
        PyObject *repr=NULL, *ascii=NULL;
        if ((repr = PyObject_Repr(inst->obj)) != NULL)
            ascii = PyUnicode_AsASCIIString(repr);
        if (repr && ascii)
            fprintf(stderr, "%s instance at 0x%p, %zd refs: %s\n", Py_TYPE(inst->obj)->tp_name, (void*)inst->obj, Py_REFCNT(inst->obj), PyBytes_AS_STRING(ascii));
        else
            fprintf(stderr, "%s instance at 0x%p, %zd refs: (repr() failed)\n", Py_TYPE(inst->obj)->tp_name, (void*)inst->obj, Py_REFCNT(inst->obj));
        Py_XDECREF(ascii);
        Py_XDECREF(repr);
    HASHMAP_END_EACH
    fprintf(stderr, "=========================================\n");
}
#if defined(IK_PYTHON_REFCOUNT_BACKTRACES)
void
ik_python_print_active_instance_backtraces(void)
{
    int i;
    struct module_state* state;
    PyObject* m = PyState_FindModule(&ik_module);
    if (m == NULL)
        return;
    state = PyModule_GetState(m);

    fprintf(stderr, "=========================================\n");
    HASHMAP_FOR_EACH(&state->active_instances, PyObject*, struct active_instance, pobj, inst)
        PyObject *repr=NULL, *ascii=NULL;
        if ((repr = PyObject_Repr(inst->obj)) != NULL)
            ascii = PyUnicode_AsASCIIString(repr);
        if (repr && ascii)
            fprintf(stderr, "%s instance at 0x%p, %zd refs: %s\n", Py_TYPE(inst->obj)->tp_name, (void*)inst->obj, Py_REFCNT(inst->obj), PyBytes_AS_STRING(ascii));
        else
            fprintf(stderr, "%s instance at 0x%p, %zd refs: (repr() failed)\n", Py_TYPE(inst->obj)->tp_name, (void*)inst->obj, Py_REFCNT(inst->obj));
        Py_XDECREF(ascii);
        Py_XDECREF(repr);

        fprintf(stderr, "Backtrace to where tp_new slot was called:\n");
        for (i = 2; i < inst->backtrace_size; ++i)
            fprintf(stderr, "    %s\n", inst->backtrace[i]);
        fprintf(stderr, "  -----------------------------------------\n");
    HASHMAP_END_EACH
    fprintf(stderr, "=========================================\n");
}
#endif
static PyObject*
ik_module_active_instances(PyObject* m, PyObject* args)
{
    struct module_state* state = PyModule_GetState(m);
    (void)args;
    return PyLong_FromLong(state->active_instance_count);
}
static PyObject*
ik_module_print_active_instances(PyObject* m, PyObject* args)
{
    (void)m; (void)args;
    ik_python_print_active_instances();
    Py_RETURN_NONE;
}
#if defined(IK_PYTHON_REFCOUNT_BACKTRACES)
static PyObject*
ik_module_print_active_instance_backtraces(PyObject* m, PyObject* args)
{
    ik_python_print_active_instance_backtraces();
    Py_RETURN_NONE;
}
#endif
#else
#   define debug_refs_inc(m, o, t) (void)m; (void)o; (void)t;
#   define debug_refs_dec(m, o, t) (void)m; (void)o; (void)t;
#endif

/* ------------------------------------------------------------------------- */
static void
public_type_instantiated_callback(PyObject* o, const char* typename)
{
    PyObject* m = PyState_FindModule(&ik_module);
    Py_INCREF(m);
    debug_refs_inc(m, o, typename);
}

/* ------------------------------------------------------------------------- */
static void
public_type_deleted_callback(PyObject* o, const char* typename)
{
    PyObject* m = PyState_FindModule(&ik_module);
    Py_DECREF(m);
    debug_refs_dec(m, o, typename);
}

/* ------------------------------------------------------------------------- */
#define X(typename)                                                           \
        static newfunc typename##_new_orig;                                   \
        static destructor typename##_dealloc_orig;                            \
        static PyObject* typename##_new_wrapper(PyTypeObject* type, PyObject* args, PyObject* kwds) \
        {                                                                     \
            PyObject* o = (*typename##_new_orig)(type, args, kwds);           \
            if (o)                                                            \
                public_type_instantiated_callback(o, #typename);              \
            return o;                                                         \
        }                                                                     \
        static void typename##_dealloc_wrapper(PyObject* o)                   \
        {                                                                     \
            public_type_deleted_callback(o, #typename);                       \
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

#if defined(IK_PYTHON_REFCOUNT_DEBUGGING)
    {
        struct module_state* state = PyModule_GetState(m);
        state->active_instance_count = 0;
        if (hashmap_init_with_options(
            &state->active_instances,
            sizeof(struct instance_key),
            sizeof(struct active_instance),
            4096,
            hash32_jenkins_oaat) != HM_OK)
        {
            goto init_module_failed;
        }
    }
#endif

    if (init_builtin_types(m) != 0)            goto init_module_failed;
    if (add_builtin_objects_to_module(m) != 0) goto init_module_failed;
    if (add_constants_to_module(m) != 0)       goto init_module_failed;

    return m;

    init_module_failed  : Py_DECREF(m);
    return NULL;
}
