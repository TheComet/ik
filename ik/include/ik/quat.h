#pragma once

#include "ik/vec3.h"

C_BEGIN

union ik_quat
{
    struct {
        ikreal x;
        ikreal y;
        ikreal z;
        ikreal w;
    } q;
    struct {
        union ik_vec3 v;
        ikreal w;
    } v;
    ikreal f[4];
};

C_END
