#include "ik/python/ik_type_Attachment.h"

#define ik_pole_CheckExact(o) \
    (Py_TYPE(o) == &ik_PoleType)

typedef struct ik_Pole
{
    ik_Attachment super;
} ik_Pole;

extern PyTypeObject ik_PoleType;

int
init_ik_PoleType(void);
