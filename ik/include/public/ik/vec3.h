#ifndef VEC3_H
#define VEC3_H

#include "ik/config.h"

C_BEGIN

struct ik_vec3_t
{
    union {
        struct {
            ikreal_t x;
            ikreal_t y;
            ikreal_t z;
        };
        ikreal_t f[3];
    };
};

IK_INTERFACE(vec3_interface)
{
    struct ik_vec3_t
    (*vec3)(ikreal_t x, ikreal_t y, ikreal_t z);

    void
    (*set)(ikreal_t v[3], const ikreal_t src[3]);

    void
    (*set_zero)(ikreal_t v[3]);

    void
    (*add_scalar)(ikreal_t v1[3], ikreal_t scalar);

    void
    (*add_vec3)(ikreal_t v1[3], const ikreal_t v2[3]);

    void
    (*sub_scalar)(ikreal_t v1[3], ikreal_t scalar);

    void
    (*sub_vec3)(ikreal_t v1[3], const ikreal_t v2[3]);

    void
    (*mul_scalar)(ikreal_t v1[3], ikreal_t scalar);

    void
    (*mul_vec3)(ikreal_t v1[3], const ikreal_t v2[3]);

    void
    (*div_scalar)(ikreal_t v[3], ikreal_t scalar);

    void
    (*div_vec3)(ikreal_t v[3], const ikreal_t v2[3]);

    ikreal_t
    (*length_squared)(const ikreal_t v[3]);

    ikreal_t
    (*length)(const ikreal_t v[3]);

    void
    (*normalize)(ikreal_t v[3]);

    ikreal_t
    (*dot)(const ikreal_t v1[3], const ikreal_t v2[3]);

    void
    (*cross)(ikreal_t v1[3], const ikreal_t v2[3]);

    void
    (*ncross)(ikreal_t v1[3], const ikreal_t v2[3]);

    /*!
     * @brief Rotations a vector by the specified quaternion.
     */
    void
    (*rotate)(ikreal_t v[3], const ikreal_t q[4]);

    void
    (*project)(ikreal_t v1[3], const ikreal_t v2[3]);

    void
    (*project_normalized)(ikreal_t v1[3], const ikreal_t v2[3]);
};


C_END

#endif /* VEC3_H */
