#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/ik.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Effector_dealloc(ik_Effector* self)
{
    IK_DECREF(self->super.attachment);
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
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
Effector_settarget_position(ik_Effector* self, PyObject* value, void* closure)
{
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    return vec3_python_to_ik(value, eff->target_position.f);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Effector_gettarget_position(ik_Effector* self, void* closure)
{
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    return (PyObject*)vec3_ik_to_python(eff->target_position.f);
}

/* ------------------------------------------------------------------------- */
static int
Effector_settarget_rotation(ik_Effector* self, PyObject* value, void* closure)
{
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
Effector_gettarget_rotation(ik_Effector* self, void* closure)
{
    struct ik_effector* eff = (struct ik_effector*)self->super.attachment;
    (void)closure;
    return (PyObject*)quat_ik_to_python(eff->target_rotation.f);
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Effector_getsetters[] = {
    {"target_position", (getter)Effector_gettarget_position, (setter)Effector_settarget_position, ""},
    {"target_rotation", (getter)Effector_gettarget_rotation, (setter)Effector_settarget_rotation, ""},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(EFFECTOR_DOC,
"");
PyTypeObject ik_EffectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Effector",
    .tp_basicsize = sizeof(ik_Effector),
    .tp_dealloc = (destructor)Effector_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = EFFECTOR_DOC,
    .tp_getset = Effector_getsetters,
    .tp_new = Effector_new
};

/* ------------------------------------------------------------------------- */
int
init_ik_EffectorType(void)
{
    if (PyType_Ready(&ik_EffectorType) < 0)
        return -1;
    return 0;
}
