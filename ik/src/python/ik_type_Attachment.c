#include "ik/python/ik_type_Attachment.h"
#include "ik/python/ik_docstrings.h"
#include "ik/bone.h"
#include "ik/attachment.h"

/* ------------------------------------------------------------------------- */
static void
Attachment_dealloc(PyObject* myself)
{
    Py_TYPE(myself)->tp_free(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Attachment_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Attachment* self;
    (void)args; (void)kwds;

    self = (ik_Attachment*)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    /*
     * self->attachment should be set by the derived type, though it's
     * possible to subclass ik.Attachment from python in which case
     * self->attachment will remain NULL. Make sure to handle this case
     * everywhere.
     */

    return (PyObject*)self;
}


/* ------------------------------------------------------------------------- */
PyTypeObject ik_AttachmentType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Attachment",
    .tp_basicsize = sizeof(ik_Attachment),
    .tp_dealloc = (destructor)Attachment_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = IK_ATTACHMENT_DOC,
    .tp_new = Attachment_new
};

/* ------------------------------------------------------------------------- */
int
init_ik_AttachmentType(void)
{
    if (PyType_Ready(&ik_AttachmentType) < 0)
        return -1;
    return 0;
}
