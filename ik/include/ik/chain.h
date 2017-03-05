#ifndef CHAIN_H
#define CHAIN_H

#include "ik/config.h"
#include "ik/segment.h"
#include "ik/ordered_vector.h"

C_HEADER_BEGIN

struct segment_t
{
    struct vector3_t position;
    struct quaternion_t rotation;

    struct vector3_t initial_position;
    struct quaternion_t initial_rotation;
};

struct chain_t
{
    struct ordered_vector_t segments;  /* number of segment_t objects */
    struct vector3_t* base_position;
    struct vector3_t* target_position;
};

C_HEADER_END

#endif /* CHAIN_H */
