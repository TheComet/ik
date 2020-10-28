#pragma once

#include "ik/config.h"

C_BEGIN

#if defined(IK_PYTHON_REFCOUNT_DEBUGGING)

IK_PUBLIC_API int
ik_python_active_instances(void);

IK_PUBLIC_API void
ik_python_print_active_instances(void);

#if defined(IK_PYTHON_REFCOUNT_BACKTRACES)

IK_PUBLIC_API void
ik_python_print_active_instance_backtraces(void);

#endif
#endif

C_END
