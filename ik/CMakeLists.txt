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

# Define special IK preprocess include files
file (GLOB IK_VTABLE_FILES include/ik/vtables/*.v)

# IK preprocessor script
set (Python_ADDITIONAL_VERSIONS 3)
find_package (PythonInterp REQUIRED)
set (IK_VTABLE_H_FILES "")
set (PYTHON_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/ik_gen_vtable.py)
macro (add_vtable_file VTABLE_FILE)
    get_filename_component (H_FILE_BASENAME ${VTABLE_FILE} NAME)
    string (REPLACE ".v" ".h" H_FILE_BASENAME ${H_FILE_BASENAME})
    set (H_FILE ${CMAKE_CURRENT_BINARY_DIR}/include/ik/${H_FILE_BASENAME})
    add_custom_command (
        OUTPUT ${H_FILE}
        DEPENDS ${VTABLE_FILE} ${PYTHON_SCRIPT}
        COMMAND ${PYTHON_EXECUTABLE} ${PYTHON_SCRIPT} ${VTABLE_FILE} ${H_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating vtable ${H_FILE}" VERBATIM)
    set (IK_VTABLE_H_FILES ${IK_VTABLE_H_FILES} ${H_FILE})
endmacro ()
foreach (VTABLE_FILE ${IK_VTABLE_FILES})
    add_vtable_file (${VTABLE_FILE})
endforeach ()

###############################################################################
# compiler flags
###############################################################################

if (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
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
file (GLOB IK_SOURCES 
    "src/*.c"
    "src/solver/*.c"
    "src/solver/BASE/*.c"
    "src/solver/ONE_BONE/*.c"
    "src/solver/TWO_BONE/*.c"
    "src/solver/FABRIK/*.c"
    "src/solver/MSS/*.c")

list (APPEND IK_HEADERS
    "include/ik/config.h.in"
    "include/ik/export.h.in")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    file (GLOB IK_PLATFORM_SOURCES "src/platform/linux/*.c")
endif ()
list (APPEND IK_SOURCES ${IK_PLATFORM_SOURCES})

include_directories ("include" "${CMAKE_CURRENT_BINARY_DIR}/include")

add_library (ik ${IK_LIB_TYPE}
    ${IK_HEADERS}
    ${IK_SOURCES}
    ${IK_VTABLE_H_FILES}
)

# The library is being built
add_definitions (-DIK_BUILDING)

target_compile_options (ik
    PUBLIC $<$<C_COMPILER_ID:MSVC>:
        /EHa /MTd /W4 /WX /wd4305 /wd4201 /wd4706 /wd4100 /wd4244 /D_CRT_SECURE_NO_DEPRECATE
    >
    PUBLIC $<$<C_COMPILER_ID:GNU>:
        -W -Wall -Wextra -Werror -Wshadow -Wconversion -Wno-unused-parameter
        -Wno-conversion -Wno-implicit-fallthrough -pedantic -pedantic-errors -fno-strict-aliasing -ffast-math
    >
    PUBLIC $<$<C_COMPILER_ID:Clang>:
        -W -Wall -Wextra -Werror -Wshadow -Wconversion -Wno-unused-parameter
        -Wno-implicit-fallthrough -Wno-conversion -pedantic -pedantic-errors -fno-strict-aliasing -ffast-math
    >
)

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

