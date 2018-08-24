#ifndef MEMORY_H
#define MEMORY_H

#include "ik/config.h"

#ifdef IK_MEMORY_DEBUGGING
#   define MALLOC malloc_wrapper
#   define FREE   free_wrapper
#else
#   include <stdlib.h>
#   define MALLOC malloc
#   define FREE   free
#endif

C_BEGIN

#if defined(IK_BUILDING)

/*!
 * @brief Initializes the memory system.
 *
 * In release mode this does nothing. In debug mode it will initialize
 * memory reports and backtraces, if enabled.
 */
IK_PRIVATE_API void
ik_memory_init(void);

/*!
 * @brief De-initializes the memory system.
 *
 * In release mode this does nothing. In debug mode this will output the memory
 * report and print backtraces, if enabled.
 * @return Returns the number of memory leaks.
 */
IK_PRIVATE_API uintptr_t
ik_memory_deinit(void);

#ifdef IK_MEMORY_DEBUGGING
/*!
 * @brief Does the same thing as a normal call to malloc(), but does some
 * additional work to monitor and track down memory leaks.
 */
IK_PRIVATE_API void*
malloc_wrapper(intptr_t size);

/*!
 * @brief Does the same thing as a normal call to fee(), but does some
 * additional work to monitor and track down memory leaks.
 */
IK_PRIVATE_API void
free_wrapper(void* ptr);
#endif /* IK_MEMORY_DEBUGGING */

IK_PRIVATE_API void
mutated_string_and_hex_dump(void* data, intptr_t size_in_bytes);

#endif /* IK_BUILDING */

C_END

#endif /* MEMORY_H */
