#include "Python.h"

struct ik_effector_t;
typedef struct ik_Vec3 ik_Vec3;
typedef struct ik_Quat ik_Quat;

typedef struct ik_Effector
{
    PyObject_HEAD
    struct ik_effector_t* effector;
    ik_Vec3* target_position;
    ik_Quat* target_rotation;
} ik_Effector;

extern PyTypeObject ik_EffectorType;

int
init_ik_EffectorType(void);
