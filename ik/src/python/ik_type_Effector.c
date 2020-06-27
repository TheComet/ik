#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/ik.h"
#include "structmember.h"

#if defined(IK_PRECISION_DOUBLE) || defined(IK_PRECISION_LONG_DOUBLE)
#   define FMT "d"
#   define MEMBER_TYPE T_DOUBLE
#elif defined(IK_PRECISION_FLOAT)
#   define FMT "f"
#   define MEMBER_TYPE T_FLOAT
#else
#   error Dont know how to wrap this precision type
#endif

/* ------------------------------------------------------------------------- */
static void
Effector_dealloc(PyObject* myself)
{
    ik_Effector* self = (ik_Effector*)myself;

    IK_DECREF(self->super.attachment);
    Py_TYPE(myself)->tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Effector* self;
    struct ik_effector* effector;

    /* Allocate effector */
    if ((effector = ik_effector_create()) == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate internal effector");
        goto alloc_effector_failed;
    }
    IK_INCREF(effector);

    self = (ik_Effector*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    self->super.attachment = (struct ik_attachment*)effector;
    return (PyObject*)self;

    alloc_self_failed     : IK_DECREF(effector);
    alloc_effector_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Effector_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    ik_Vec3* target_position = NULL;
    ik_Quat* target_rotation = NULL;
    int weight_nlerp = -1;
    int keep_global_orientation = -1;

    static char* kwds_str[] = {
        "chain_length",
        "target_position",
        "target_rotation",
        "weight",
        "rotation_weight",
        "rotation_decay",
        "weight_nlerp",
        "keep_global_orientation",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|HO!O!" FMT FMT FMT "pp", kwds_str,
                                     &eff->chain_length,
                                     &ik_Vec3Type, &target_position,
                                     &ik_QuatType, &target_rotation,
                                     &eff->weight,
                                     &eff->rotation_weight,
                                     &eff->rotation_decay,
                                     &weight_nlerp,
                                     &keep_global_orientation))
    {
        return -1;
    }

    if (target_position)
        ik_vec3_copy(eff->target_position.f, target_position->vec.f);
    if (target_rotation)
        ik_quat_copy(eff->target_rotation.f, target_rotation->quat.f);

    if (weight_nlerp == 0)
        eff->features &= ~IK_EFFECTOR_WEIGHT_NLERP;
    if (weight_nlerp == 1)
        eff->features |= IK_EFFECTOR_WEIGHT_NLERP;

    if (keep_global_orientation == 0)
        eff->features &= ~IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION;
    if (keep_global_orientation == 1)
        eff->features |= IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION;

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_getchain_length(PyObject* myself, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    return PyLong_FromLong(eff->chain_length);
}
static int
Effector_setchain_length(PyObject* myself, PyObject* value, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    unsigned long v = PyLong_AsUnsignedLongMask(value);
    (void)closure;
    if (v == (unsigned long)-1 && PyErr_Occurred())
        return -1;

    eff->chain_length = (uint16_t)v;
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_gettarget_position(PyObject* myself, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    return (PyObject*)vec3_ik_to_python(eff->target_position.f);
}
static int
Effector_settarget_position(PyObject* myself, PyObject* value, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    if (!ik_Vec3_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type");
        return -1;
    }

    ik_vec3_copy(eff->target_position.f, ((ik_Vec3*)value)->vec.f);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_gettarget_rotation(PyObject* myself, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    return (PyObject*)quat_ik_to_python(eff->target_rotation.f);
}
static int
Effector_settarget_rotation(PyObject* myself, PyObject* value, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    if (!ik_Quat_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Quat() type");
        return -1;
    }

    ik_quat_copy(eff->target_rotation.f, ((ik_Quat*)value)->quat.f);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_getweight(PyObject* myself, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    return PyFloat_FromDouble(eff->weight);
}
static int
Effector_setweight(PyObject* myself, PyObject* value, void* closure)
{
    PyObject* as_float;
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;

    if ((as_float = PyNumber_Float(value)) == NULL)
        return -1;

    eff->weight = PyFloat_AS_DOUBLE(as_float);
    Py_DECREF(as_float);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_getrotation_weight(PyObject* myself, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    return PyFloat_FromDouble(eff->rotation_weight);
}
static int
Effector_setrotation_weight(PyObject* myself, PyObject* value, void* closure)
{
    PyObject* as_float;
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;

    if ((as_float = PyNumber_Float(value)) == NULL)
        return -1;

    eff->rotation_weight = PyFloat_AS_DOUBLE(as_float);
    Py_DECREF(as_float);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_getrotation_decay(PyObject* myself, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    return PyFloat_FromDouble(eff->rotation_decay);
}
static int
Effector_setrotation_decay(PyObject* myself, PyObject* value, void* closure)
{
    PyObject* as_float;
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;

    if ((as_float = PyNumber_Float(value)) == NULL)
        return -1;

    eff->rotation_decay = PyFloat_AS_DOUBLE(as_float);
    Py_DECREF(as_float);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
get_feature_flag(struct ik_effector* eff, enum ik_effector_features feature)
{
    return PyBool_FromLong(eff->features & feature);
}
static int
set_feature_flag(struct ik_effector* eff, PyObject* value, enum ik_effector_features feature)
{
    int is_true = PyObject_IsTrue(value);
    if (is_true == -1)
        return -1;

    if (is_true)
        eff->features |= feature;
    else
        eff->features &= ~feature;
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_getweight_nlerp(PyObject* myself, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    (void)closure;
    return get_feature_flag((struct ik_effector*)self->super.attachment, IK_EFFECTOR_WEIGHT_NLERP);
}
static int
Effector_setweight_nlerp(PyObject* myself, PyObject* value, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    (void)closure;
    return set_feature_flag((struct ik_effector*)self->super.attachment, value, IK_EFFECTOR_WEIGHT_NLERP);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_getkeep_global_orientation(PyObject* myself, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    (void)closure;
    return get_feature_flag((struct ik_effector*)self->super.attachment, IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION);
}
static int
Effector_setkeep_global_orientation(PyObject* myself, PyObject* value, void* closure)
{
    ik_Effector* self = (ik_Effector*)myself;
    (void)closure;
    return set_feature_flag((struct ik_effector*)self->super.attachment, value, IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION);
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Effector_getsetters[] = {
    {"chain_length",            Effector_getchain_length,            Effector_setchain_length,            ""},
    {"target_position",         Effector_gettarget_position,         Effector_settarget_position,         ""},
    {"target_rotation",         Effector_gettarget_rotation,         Effector_settarget_rotation,         ""},
    {"weight",                  Effector_getweight,                  Effector_setweight,                  ""},
    {"rotation_weight",         Effector_getrotation_weight,         Effector_setrotation_weight,         ""},
    {"rotation_decay",          Effector_getrotation_decay,          Effector_setrotation_decay,          ""},
    {"weight_nlerp",            Effector_getweight_nlerp,            Effector_setweight_nlerp,            ""},
    {"keep_global_orientation", Effector_getkeep_global_orientation, Effector_setkeep_global_orientation, ""},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_repr_build_arglist_list(PyObject* myself)
{
    ik_Effector* self = (ik_Effector*)myself;
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;

    PyObject* args = PyList_New(0);
    if (args == NULL)
        return NULL;

    /* Chain length */
    if (eff->chain_length != 0)
    {
        int append_result;
        PyObject* arg = PyUnicode_FromFormat("chain_length=%d", (int)eff->chain_length);
        if (arg == NULL)
            goto addarg_failed;
        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Target position */
    {
        int append_result;
        PyObject* target_position;
        PyObject* arg;

        target_position = (PyObject*)vec3_ik_to_python(eff->target_position.f);
        if (target_position == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("target_position=%R", target_position);
        Py_DECREF(target_position);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Target rotation */
    {
        int append_result;
        PyObject* target_rotation;
        PyObject* arg;

        target_rotation = (PyObject*)quat_ik_to_python(eff->target_rotation.f);
        if (target_rotation == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("target_rotation=%R", target_rotation);
        Py_DECREF(target_rotation);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Weight */
    if (eff->weight != 1.0)
    {
        int append_result;
        PyObject* weight;
        PyObject* arg;

        weight = PyFloat_FromDouble(eff->weight);
        if (weight == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("weight=%R", weight);
        Py_DECREF(weight);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* rotation weight */
    if (eff->rotation_weight != 1.0)
    {
        int append_result;
        PyObject* rotation_weight;
        PyObject* arg;

        rotation_weight = PyFloat_FromDouble(eff->rotation_weight);
        if (rotation_weight == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("rotation_weight=%R", rotation_weight);
        Py_DECREF(rotation_weight);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* rotation decay */
    if (eff->rotation_decay != 0.25)
    {
        int append_result;
        PyObject* rotation_decay;
        PyObject* arg;

        rotation_decay = PyFloat_FromDouble(eff->rotation_decay);
        if (rotation_decay == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("rotation_decay=%R", rotation_decay);
        Py_DECREF(rotation_decay);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* weight nlerp feature */
    if (eff->features & IK_EFFECTOR_WEIGHT_NLERP)
    {
        int append_result;
        PyObject* arg = PyUnicode_FromString("chain_length=True");
        if (arg == NULL)
            goto addarg_failed;
        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* keep global orientation feature */
    if (eff->features & IK_EFFECTOR_KEEP_GLOBAL_ORIENTATION)
    {
        int append_result;
        PyObject* arg = PyUnicode_FromString("keep_global_orientation=True");
        if (arg == NULL)
            goto addarg_failed;
        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    return args;

    addarg_failed : Py_DECREF(args);
    return NULL;
}
static PyObject*
Effector_repr_build_arglist_string(PyObject* myself)
{
    PyObject* separator;
    PyObject* arglist;
    PyObject* string;

    separator = PyUnicode_FromString(", ");
    if (separator == NULL)
        return NULL;

    arglist = Effector_repr_build_arglist_list(myself);
    if (arglist == NULL)
    {
        Py_DECREF(separator);
        return NULL;
    }

    string = PyUnicode_Join(separator, arglist);
    Py_DECREF(separator);
    Py_DECREF(arglist);
    return string;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_repr(PyObject* myself)
{
    PyObject* repr;
    PyObject* argstring = Effector_repr_build_arglist_string(myself);
    if (argstring == NULL)
        return NULL;

    repr = PyUnicode_FromFormat("ik.Effector(%U)", argstring);
    Py_DECREF(argstring);
    return repr;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_str(PyObject* myself)
{
    return Effector_repr(myself);
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(EFFECTOR_DOC,
"");
PyTypeObject ik_EffectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Effector",
    .tp_basicsize = sizeof(ik_Effector),
    .tp_dealloc = Effector_dealloc,
    .tp_repr = Effector_repr,
    .tp_str = Effector_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = EFFECTOR_DOC,
    .tp_getset = Effector_getsetters,
    .tp_new = Effector_new,
    .tp_init = Effector_init
};

/* ------------------------------------------------------------------------- */
int
init_ik_EffectorType(void)
{
    ik_EffectorType.tp_base = &ik_AttachmentType;
    if (PyType_Ready(&ik_EffectorType) < 0)
        return -1;
    return 0;
}
