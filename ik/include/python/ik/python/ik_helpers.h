#pragma once

#include "ik/config.h"

#if defined(IK_PRECISION_DOUBLE) || defined(IK_PRECISION_LONG_DOUBLE)
#   define FMT "d"
#   define MEMBER_TYPE T_DOUBLE
#elif defined(IK_PRECISION_FLOAT)
#   define FMT "f"
#   define MEMBER_TYPE T_FLOAT
#else
#   error Dont know how to wrap this precision type
#endif

#define REF_VEC3_DATA(dstvec, dstvecdata) do {                                \
    /* copy current value of vector to foreign data */                        \
    ik_vec3_copy((dstvecdata)->f, (dstvec)->vecref->f);                       \
    /* ref foreign data */                                                    \
    (dstvec)->vecref = (dstvecdata);                                          \
} while(0)

#define UNREF_VEC3_DATA(dstvec) do {                                          \
    /* copy foreign data to own data */                                       \
    ik_vec3_copy((dstvec)->vec.f, (dstvec)->vecref->f);                       \
    /* ref to own data */                                                     \
    (dstvec)->vecref = &(dstvec)->vec;                                        \
} while(0)

#define REF_QUAT_DATA(dstquat, dstquatdata) do {                              \
    /* copy current value of quaternion to foreign data */                    \
    ik_quat_copy((dstquatdata)->f, (dstquat)->quatref->f);                    \
    /* ref foreign data */                                                    \
    (dstquat)->quatref = (dstquatdata);                                       \
} while(0)

#define UNREF_QUAT_DATA(dstquat) do {                                         \
    /* copy foreign data to own data */                                       \
    ik_quat_copy((dstquat)->quat.f, (dstquat)->quatref->f);                   \
    /* ref to own data */                                                     \
    (dstquat)->quatref = &(dstquat)->quat;                                    \
} while(0)

#define ASSIGN_VEC3(dstvec, srcvec) do {                                      \
    ik_Vec3* tmp;                                                             \
    union ik_vec3* vecdata = (dstvec)->vecref;                                \
    UNREF_VEC3_DATA(dstvec);                                                  \
                                                                              \
    /* Swap in new vector */                                                  \
    tmp = (dstvec);                                                           \
    Py_INCREF(srcvec);                                                        \
    (dstvec) = (srcvec);                                                      \
    Py_DECREF(tmp);                                                           \
                                                                              \
    REF_VEC3_DATA(dstvec, vecdata);                                           \
} while(0)

#define ASSIGN_QUAT(dstquat, srcquat) do {                                    \
    ik_Quat* tmp;                                                             \
    union ik_quat* quatdata = (dstquat)->quatref;                             \
    UNREF_QUAT_DATA(dstquat);                                                 \
                                                                              \
    /* Swap in new quaternion */                                              \
    tmp = (dstquat);                                                          \
    Py_INCREF(srcquat);                                                       \
    (dstquat) = (srcquat);                                                    \
    Py_DECREF(tmp);                                                           \
                                                                              \
    REF_QUAT_DATA(dstquat, quatdata);                                         \
} while(0)
