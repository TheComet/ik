#ifndef IK_BUILD_INFO_H
#define IK_BUILD_INFO_H

#include "ik/config.h"

C_BEGIN

IK_INTERFACE(build_info_interface)
{
    const char*
    (*version)(void);

    int
    (*build_number)(void);

    const char*
    (*host)(void);

    const char*
    (*date)(void);

    const char*
    (*commit)(void);

    const char*
    (*compiler)(void);

    const char*
    (*cmake)(void);

    const char*
    (*all)(void);
};

C_END

#endif /* IK_BUILD_INFO_H */
