#include "ik/python/ik_type_Attachment.h"
#include "ik/python/ik_type_Node.h"
#include "ik/node.h"
#include "ik/attachment.h"

/* ------------------------------------------------------------------------- */
static void
Attachment_dealloc(ik_Attachment* self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
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
PyDoc_STRVAR(ATTACHMENT_NODE_DOC, "");
static PyObject*
Attachment_getnode(ik_Attachment* self, void* closure)
{
    (void)closure;

    if (self->attachment && self->attachment->node)
    {
        ik_Node* node = self->attachment->node->user.ptr;
        return Py_INCREF(node), (PyObject*)node;
    }

    PyErr_SetString(PyExc_RuntimeError, "Attachment is not attached to any node");
    return NULL;
}
static int
Attachment_setnode(ik_Attachment* self, PyObject* value, void* closure)
{
    (void)self; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "Node is read-only. Use node.attach() to attach to a node.");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Node_getset[] = {
    {"node", (getter)Attachment_getnode, (setter)Attachment_setnode, ATTACHMENT_NODE_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_DOC,
"");
PyTypeObject ik_AttachmentType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Attachment",
    .tp_basicsize = sizeof(ik_Attachment),
    .tp_dealloc = (destructor)Attachment_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = NODE_DOC,
    .tp_getset = Node_getset,
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
