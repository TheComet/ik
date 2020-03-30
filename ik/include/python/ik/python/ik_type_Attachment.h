#ifndef IK_PYTHON_TYPE_ATTACHMENT_H
#define IK_PYTHON_TYPE_ATTACHMENT_H

#include "Python.h"

struct ik_attachment;

typedef struct ik_Attachment
{
    PyObject_HEAD
    struct ik_attachment* attachment;
} ik_Attachment;

extern PyTypeObject ik_AttachmentType;

int
init_ik_AttachmentType(void);

#endif /* IK_PYTHON_TYPE_ATTACHMENT_H */
