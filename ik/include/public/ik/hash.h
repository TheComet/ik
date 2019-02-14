#ifndef WAVESIM_HASH_H
#define WAVESIM_HASH_H

#include "ik/config.h"

C_BEGIN

typedef uint32_t hash32_t;
typedef hash32_t (*hash32_func)(const void*, uintptr_t);

IK_PRIVATE_API hash32_t
hash32_jenkins_oaat(const void* key, uintptr_t len);

/*!
 * @brief Taken from boost::hash_combine. Combines two hash values into a
 * new hash value.
 */
IK_PRIVATE_API hash32_t
hash32_combine(hash32_t lhs, hash32_t rhs);

IK_PRIVATE_API hash32_t
hash32_vec3(const ikreal_t v[3]);

C_END

#endif /* WAVESIM_HASH_H */
