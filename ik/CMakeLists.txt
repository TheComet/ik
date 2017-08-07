include (CheckIncludeFiles)
include (CheckCSourceCompiles)

# memory debugging
if (${CMAKE_BUILD_TYPE} MATCHES Debug)
	set (IK_MEMORY_DEBUGGING_DEFAULT ON)
else ()
	set (IK_MEMORY_DEBUGGING_DEFAULT OFF)
endif ()
if (IK_MEMORY_DEBUGGING AND NOT WIN32 AND NOT CYGWIN)
	if (WIN32 OR CYGWIN)
		set (IK_MEMORY_BACKTRACE_DEFAULT OFF)
	else ()
		set (IK_MEMORY_BACKTRACE_DEFAULT ON)
	endif ()
endif ()
option (IK_MEMORY_DEBUGGING "Global switch for memory options. Keeps track of the number of allocations and de-allocations and prints a report when the program shuts down" ${IK_MEMORY_DEBUGGING_DEFAULT})
option (IK_MEMORY_BACKTRACE "Generates backtraces for every malloc(), making it easy to track down memory leaks" ${IK_MEMORY_BACKTRACE_DEFAULT})

# Need to set IK_PLATFORM for dllimport/dllexport
if (WIN32)
    set (IK_PLATFORM "WINDOWS")
elseif (APPLE AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (IK_PLATFORM "OSX")
elseif (IOS)
    set (IK_PLATFORM "IOS")
elseif (UNIX)
    set (IK_PLATFORM "LINUX")
else ()
    set (IK_PLATFORM "ANDROID")
endif ()

# Enable restrict keyword in quaternion and vector operations if not in debug
# Only do this if IK_RESTRICT is not cached yet.
get_cmake_property (CACHED_VARS VARIABLES)
list (FIND CACHED_VARS "IK_RESTRICT" RESULT)
if (${RESULT} MATCHES -1)
    foreach (RESTRICT_KEYWORD restrict __restrict __restrict__)
        check_c_source_compiles ("int test (void *${RESTRICT_KEYWORD} x); int main (void) {return 0;}" IK_RESTRICT_${RESTRICT_KEYWORD})
        if (IK_RESTRICT_${RESTRICT_KEYWORD})
            set (IK_RESTRICT ${RESTRICT_KEYWORD})
            break ()
        endif ()
    endforeach ()
    set (IK_RESTRICT ${IK_RESTRICT} CACHE STRING "Restrict Keyword (may be empty)")
endif ()

# Use stdint.h if possible, otherwise use pstdint.h
check_include_files ("stdint.h" IK_HAVE_STDINT_H)

set (IK_REAL "float" CACHE STRING "Type to use for real numbers")
option (IK_DOT_OUTPUT "When enabled, the generated chains are dumped to DOT for debug purposes" OFF)

set (IK_LIB_TYPE "STATIC" CACHE STRING "Shared or static library")

configure_file ("include/ik/export.h.in"
                "include/ik/export.h")
configure_file ("include/ik/config.h.in"
                "include/ik/config.h")

include_directories ("${CMAKE_CURRENT_BINARY_DIR}/include")

###############################################################################
# compiler flags
###############################################################################

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
    add_definitions (-W -Wall -Wextra -Werror -pedantic -Wno-unused-parameter -fno-math-errno -ffast-math)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
    add_definitions (-W -Wall -Wextra -Werror -pedantic -Wno-unused-parameter -fno-math-errno -ffast-math)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "Intel")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
    add_definitions (-D_CRT_SECURE_NO_WARNINGS)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        STRING(REGEX REPLACE "/MDd" "" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
        add_definitions (-MTd)
    elseif (CMAKE_BUILD_TYPE MATCHES Release)
        STRING(REGEX REPLACE "/MD" "" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
        add_definitions (-MT)
    endif ()
endif ()

###############################################################################
# source files and library definition
###############################################################################

file (GLOB IK_HEADERS "include/ik/*.h")
file (GLOB IK_SOURCES "src/*.c")

list (APPEND IK_HEADERS
    "include/ik/config.h.in"
    "include/ik/export.h.in")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    file (GLOB IK_PLATFORM_SOURCES "src/platform/linux/*.c")
endif ()
list (APPEND IK_SOURCES ${IK_PLATFORM_SOURCES})

include_directories ("include")

add_library (ik ${IK_LIB_TYPE}
    ${IK_HEADERS}
    ${IK_SOURCES}
)

# The library is being built
add_definitions (-DIK_BUILDING)

###############################################################################
# install targets
###############################################################################

install (
    TARGETS ik
    LIBRARY DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    ARCHIVE DESTINATION "lib"
)
install (
    DIRECTORY
        "include/"
        "${CMAKE_CURRENT_BINARY_DIR}/include/"
    DESTINATION "include"
    FILES_MATCHING PATTERN "*.h"
)
