#ifndef IK_MAT3X3_H
#define IK_MAT3X3_H

#include "ik/config.h"

C_BEGIN

struct ik_mat3x3_t
{
    union {
        /*! Access individual elements */
        struct {
            ikreal_t m00, m01, m02;
            ikreal_t m10, m11, m12;
            ikreal_t m20, m21, m22;
        };
        /*! Access basis vectors */
        struct {
            ikreal_t ex[3];
            ikreal_t ey[3];
            ikreal_t ez[3];
        };
        /*! Access as contiguous array */
        ikreal_t f[9];
    };
};

#if defined(IK_BUILDING)

IK_PRIVATE_API void
ik_mat3x3_from_quat(ikreal_t m[9], const ikreal_t q[4]);

#endif /* IK_BUILDING */

C_END

#endif /* IK_MAT3X3_H */
