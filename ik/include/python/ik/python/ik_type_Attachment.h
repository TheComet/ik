#pragma once

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
