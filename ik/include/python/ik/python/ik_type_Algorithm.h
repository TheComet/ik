#ifndef IK_PYTHON_TYPE_ALGORITHM_H
#define IK_PYTHON_TYPE_ALGORITHM_H

#include "ik/python/ik_type_Attachment.h"

typedef struct ik_Algorithm
{
    ik_Attachment super;
} ik_Algorithm;

extern PyTypeObject ik_AlgorithmType;

int
init_ik_AlgorithmType(void);

#endif /* IK_PYTHON_TYPE_ALGORITHM_H */
