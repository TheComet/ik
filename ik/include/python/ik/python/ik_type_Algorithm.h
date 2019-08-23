#ifndef IK_PYTHON_TYPE_ALGORITHM_H
#define IK_PYTHON_TYPE_ALGORITHM_H

#include "Python.h"

typedef struct ik_Algorithm
{
    PyObject_HEAD
} ik_Algorithm;

extern PyTypeObject ik_AlgorithmType;

int
init_ik_AlgorithmType(void);

#endif /* IK_PYTHON_TYPE_ALGORITHM_H */
