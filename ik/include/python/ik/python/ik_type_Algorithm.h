#include "ik/python/ik_type_Attachment.h"

#define ik_algorithm_CheckExact(o) \
    (Py_TYPE(o) == &ik_AlgorithmType)

typedef struct ik_Algorithm
{
    ik_Attachment super;
} ik_Algorithm;

extern PyTypeObject ik_AlgorithmType;

int
init_ik_AlgorithmType(void);
