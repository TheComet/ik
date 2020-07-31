#include "ik/python/ik_module.h"
#include "ik/python/ik_type_ModuleRef.h"
#include "ik/python/ik_docstrings.h"

/* ------------------------------------------------------------------------- */
static void
ModuleRef_dealloc(PyObject* self)
{
    Py_DECREF(((ik_ModuleRef*)self)->module);
    Py_TYPE(self)->tp_free(self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
ModuleRef_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    PyObject* module;
    ik_ModuleRef* self;
    (void)args; (void)kwds;

    module = PyState_FindModule(&ik_module);
    if (module == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to get ik module");
        return NULL;
    }

    self = (ik_ModuleRef*)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    Py_INCREF(module);
    self->module = module;

    return (PyObject*)self;
}

/* ------------------------------------------------------------------------- */
PyTypeObject ik_ModuleRefType = {
    .tp_name = "ik.ModuleRef",
    .tp_basicsize = sizeof(ik_ModuleRef),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = IK_MODULE_REF_DOC,
    .tp_dealloc = ModuleRef_dealloc,
    .tp_new = ModuleRef_new
};

/* ------------------------------------------------------------------------- */
int
init_ik_ModuleRefType(void)
{
    if (PyType_Ready(&ik_ModuleRefType) < 0)
        return -1;
    return 0;
}
