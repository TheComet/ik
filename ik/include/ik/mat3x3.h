#pragma once

#include "ik/config.h"

C_BEGIN

union ik_mat3x3_t {
    /*! Access individual elements */
    struct {
        ikreal_t m00, m01, m02;
        ikreal_t m10, m11, m12;
        ikreal_t m20, m21, m22;
    } m;
    /*! Access basis vectors */
    struct {
        ikreal_t x[3];
        ikreal_t y[3];
        ikreal_t z[3];
    } e;
    /*! Access as contiguous array */
    ikreal_t f[9];
};

#if defined(IK_BUILDING)

IK_PRIVATE_API void
ik_mat3x3_from_quat(ikreal_t m[9], const ikreal_t q[4]);

#endif /* IK_BUILDING */

C_END
