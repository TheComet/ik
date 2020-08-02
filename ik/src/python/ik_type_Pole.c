#include "ik/python/ik_type_Pole.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_helpers.h"
#include "ik/python/ik_docstrings.h"
#include "ik/pole.h"
#include "ik/vec3.inl"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Pole_dealloc(PyObject* myself)
{
    ik_Pole* self = (ik_Pole*)myself;

    UNREF_VEC3_DATA(self->position);
    Py_DECREF(self->position);
    IK_DECREF(self->super.attachment);
    ik_PoleType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Pole_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Pole* self;
    ik_Vec3* position;
    struct ik_pole* pole;

    /* Allocate internal pole */
    if ((pole = ik_pole_create()) == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate internal pole");
        goto alloc_pole_failed;
    }
    IK_INCREF(pole);

    /* Allocate members */
    position = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL);
    if (position == NULL)
        goto alloc_target_position_failed;

    /* Allocate self */
    self = (ik_Pole*)ik_PoleType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    self->super.attachment = (struct ik_attachment*)pole;
    self->position = position;

    REF_VEC3_DATA(self->position, &pole->position);

    return (PyObject*)self;

    alloc_self_failed            : Py_DECREF(position);
    alloc_target_position_failed : IK_DECREF(pole);
    alloc_pole_failed            : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Pole_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Pole* self = (ik_Pole*)myself;
    struct ik_pole* pole = (struct ik_pole*)self->super.attachment;
    ik_Vec3* position = NULL;

    static char* kwds_str[] = {
        "position",
        "angle",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!" FMT, kwds_str,
                                     &ik_Vec3Type, &position,
                                     &pole->angle))
    {
        return -1;
    }

    if (position)
        ASSIGN_VEC3(self->position, position);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Pole_getposition(PyObject* myself, void* closure)
{
    ik_Pole* self = (ik_Pole*)myself;
    (void)closure;
    return Py_INCREF(self->position), (PyObject*)self->position;
}
static int
Pole_setposition(PyObject* myself, PyObject* value, void* closure)
{
    ik_Pole* self = (ik_Pole*)myself;
    (void)closure;
    if (!ik_Vec3_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type");
        return -1;
    }

    ASSIGN_VEC3(self->position, (ik_Vec3*)value);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Pole_getangle(PyObject* myself, void* closure)
{
    ik_Pole* self = (ik_Pole*)myself;
    struct ik_pole* pole = (struct ik_pole*)self->super.attachment;
    (void)closure;
    return PyFloat_FromDouble(pole->angle);
}
static int
Pole_setangle(PyObject* myself, PyObject* value, void* closure)
{
    PyObject* as_float;
    ik_Pole* self = (ik_Pole*)myself;
    struct ik_pole* pole = (struct ik_pole*)self->super.attachment;
    (void)closure;

    if ((as_float = PyNumber_Float(value)) == NULL)
        return -1;

    pole->angle = PyFloat_AS_DOUBLE(as_float);
    Py_DECREF(as_float);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Pole_getset[] = {
    {"position", Pole_getposition, Pole_setposition, IK_POLE_POSITION_DOC},
    {"angle",    Pole_getangle,    Pole_setangle,    IK_POLE_ANGLE_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Pole_repr_build_arglist_list(PyObject* myself)
{
    ik_Pole* self = (ik_Pole*)myself;
    struct ik_pole* pole = (struct ik_pole*)self->super.attachment;

    PyObject* args = PyList_New(0);
    if (args == NULL)
        return NULL;

    /* Target position */
    {
        int append_result;
        PyObject* position;
        PyObject* arg;

        position = (PyObject*)vec3_ik_to_python(pole->position.f);
        if (position == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("position=%R", position);
        Py_DECREF(position);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Angle */
    {
        int append_result;
        PyObject* angle;
        PyObject* arg;

        angle = PyFloat_FromDouble(pole->angle);
        if (angle == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("angle=%R", angle);
        Py_DECREF(angle);
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
Pole_repr_build_arglist_string(PyObject* myself)
{
    PyObject* separator;
    PyObject* arglist;
    PyObject* string;

    separator = PyUnicode_FromString(", ");
    if (separator == NULL)
        return NULL;

    arglist = Pole_repr_build_arglist_list(myself);
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
static PyObject*
Pole_repr(PyObject* myself)
{
    PyObject* repr;
    PyObject* argstring = Pole_repr_build_arglist_string(myself);
    if (argstring == NULL)
        return NULL;

    repr = PyUnicode_FromFormat("ik.Pole(%U)", argstring);
    Py_DECREF(argstring);
    return repr;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Pole_str(PyObject* myself)
{
    return Pole_repr(myself);
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_PoleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Pole",
    .tp_basicsize = sizeof(ik_Pole),
    .tp_dealloc = Pole_dealloc,
    .tp_repr = Pole_repr,
    .tp_str = Pole_str,
    .tp_flags = Py_TPFLAGS_DEFAULT & Py_TPFLAGS_BASETYPE,
    .tp_doc = IK_POLE_DOC,
    .tp_getset = Pole_getset,
    .tp_init = Pole_init,
    .tp_new = Pole_new
};

/* ------------------------------------------------------------------------- */
static int
GenericPole_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Pole* self = (ik_Pole*)myself;
    struct ik_pole* pole = (struct ik_pole*)self->super.attachment;

    int result = ik_GenericPoleType.tp_base->tp_init(myself, args, kwds);
    if (result != 0)
        return result;

    ik_pole_set_generic(pole);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_GenericPoleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.GenericPole",
    .tp_basicsize = sizeof(ik_Pole),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_GENERICPOLE_DOC,
    .tp_base = &ik_PoleType,
    .tp_init = GenericPole_init
};

/* ------------------------------------------------------------------------- */
static int
BlenderPole_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Pole* self = (ik_Pole*)myself;
    struct ik_pole* pole = (struct ik_pole*)self->super.attachment;

    int result = ik_BlenderPoleType.tp_base->tp_init(myself, args, kwds);
    if (result != 0)
        return result;

    ik_pole_set_blender(pole);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_BlenderPoleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.BlenderPole",
    .tp_basicsize = sizeof(ik_Pole),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_BLENDERPOLE_DOC,
    .tp_base = &ik_PoleType,
    .tp_init = BlenderPole_init
};

/* ------------------------------------------------------------------------- */
static int
MayaPole_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Pole* self = (ik_Pole*)myself;
    struct ik_pole* pole = (struct ik_pole*)self->super.attachment;

    int result = ik_MayaPoleType.tp_base->tp_init(myself, args, kwds);
    if (result != 0)
        return result;

    ik_pole_set_blender(pole);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_MayaPoleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.MayaPole",
    .tp_basicsize = sizeof(ik_Pole),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_MAYAPOLE_DOC,
    .tp_base = &ik_PoleType,
    .tp_init = MayaPole_init
};

/* ------------------------------------------------------------------------- */
int
init_ik_PoleType(void)
{
    ik_PoleType.tp_base = &ik_ModuleRefType;

    if (PyType_Ready(&ik_PoleType) < 0)        return -1;
    if (PyType_Ready(&ik_GenericPoleType) < 0) return -1;
    if (PyType_Ready(&ik_BlenderPoleType) < 0) return -1;
    if (PyType_Ready(&ik_MayaPoleType) < 0)    return -1;
    return 0;
}
