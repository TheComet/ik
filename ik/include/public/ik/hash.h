#ifndef IK_HASH_H
#define IK_HASH_H

#include "ik/config.h"
#include "cstructures/hash.h"

C_BEGIN

IK_PRIVATE_API hash32_t
hash32_vec3(const ikreal_t v[3]);

C_END

#endif /* IK_HASH_H */
