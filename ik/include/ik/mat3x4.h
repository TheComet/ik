#pragma once

#include "ik/config.h"
#include "ik/vec3.h"

C_BEGIN

union ik_mat3x4
{
    /*! Access individual elements */
    struct {
        ikreal m00, m10, m20;
        ikreal m01, m11, m21;
        ikreal m02, m12, m22;
        ikreal m03, m13, m23;
    } m;
    /*! Access basis vectors */
    struct {
        union ik_vec3 ex;
        union ik_vec3 ey;
        union ik_vec3 ez;
        union ik_vec3 et;
    } e;
    /*! Access as contiguous array */
    ikreal f[12];
};

C_END
