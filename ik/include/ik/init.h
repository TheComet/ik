#ifndef IK_INIT_H
#define IK_INIT_H

#include "ik/config.h"

C_BEGIN

/*!
 * @brief Initializes the library. Must be called before using any other ik
 * function.
 * @note Calls are refcounted, i.e. for every successful call to ik_init() you
 * must call ik_deinit(). The "last" call to ik_deinit() will deinit the
 * library.
 * @return Returns IK_OK on success.
 */
IK_PUBLIC_API int ik_init(void);

/*!
 * @brief Deinitializes the library. Must be called when the library is no
 * longer needed.
 *
 * @return In debug mode, assuming CSTRUCTURES_MEMORY_DEBUGGING is enabled, the
 * call to this function that reduces the refcount to 0 will return the number
 * of memory leaks detected within the ik library. Memory leaks are tracked by
 * matching every malloc() call to a free() call. Unmatched calls are reported
 * to the log and, if CSTRUCTURES_MEMORY_BACKTRACE is enabled, a stack trace to
 * where each object was allocated will be generated. It is recommended to
 * check the return value in debug mode.
 *
 * In the case that you do get leak reports, ensure you haven't forgotten to
 * delete objects anywhere, and then submit a bug report if you believe you are
 * in the right.
 * https://github.com/thecomet/ik/issues
 *
 * If CSTRUCTURES_MEMORY_DEBUGGING is disabled, 0 is returned (default in
 * release mode).
 *
 * All other calls return 0.
 */
IK_PUBLIC_API uintptr_t ik_deinit(void);

C_END

#endif /* IK_INIT_H */

