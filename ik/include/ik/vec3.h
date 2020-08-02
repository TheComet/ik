#pragma once

#include "ik/config.h"

C_BEGIN

union ik_vec3
{
    struct {
        ikreal x;
        ikreal y;
        ikreal z;
    } v;
    ikreal f[3];
};

C_END
