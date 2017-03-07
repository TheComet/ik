set (PROJECT_NAME "IK")
set (BUILD_TYPE "STATIC")

# memory debugging
if (CMAKE_BUILD_TYPE MATCHES Debug)
    option (IK_MEMORY_DEBUGGING "Global switch for memory options. Keeps track of the number of allocations and de-allocations and prints a report when the program shuts down" ON)
else ()
    option (IK_MEMORY_DEBUGGING "Global switch for memory options. Keeps track of the number of allocations and de-allocations and prints a report when the program shuts down" OFF)
endif ()
if (IK_MEMORY_DEBUGGING)
    if(${CMAKE_BUILD_TYPE} MATCHES Debug)
        option (IK_MEMORY_BACKTRACE "Generates backtraces for every malloc(), making it easy to track down memory leaks" ON)
    else ()
        option (IK_MEMORY_BACKTRACE "Generates backtraces for every malloc(), making it easy to track down memory leaks" OFF)
    endif ()
else ()
    option (IK_MEMORY_BACKTRACE "Generates backtraces for every malloc(), making it easy to track down memory leaks" OFF)
endif ()

set (IK_REAL "float" CACHE STRING "Type to use for real numbers")

configure_file ("${EXPORT_H_TEMPLATE}"
                "include/ik/export.h")
configure_file ("include/ik/config.h.in"
                "include/ik/config.h")

include_directories ("${CMAKE_CURRENT_BINARY_DIR}/include")

###############################################################################
# source files and library definition
###############################################################################

file (GLOB IK_HEADERS "include/ik/*.h")
file (GLOB IK_SOURCES "src/*.c")

list (APPEND IK_HEADERS
    "include/ik/config.h.in"
    "${EXPORT_H_TEMPLATE}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    file (GLOB IK_PLATFORM_SOURCES "src/platform/linux/*.c")
endif ()
list (APPEND IK_SOURCES ${IK_PLATFORM_SOURCES})

include_directories ("include")

add_library (ik ${BUILD_TYPE}
    ${IK_HEADERS}
    ${IK_SOURCES}
)

# The library is being built
add_definitions (-DIK_BUILDING)

###############################################################################
# install targets
###############################################################################

# only install if shared library
if (${BUILD_TYPE} STREQUAL "SHARED")
    install (
        TARGETS
            ik
        LIBRARY DESTINATION
            "lib"
        RUNTIME DESTINATION
            "bin"
    )
endif ()
