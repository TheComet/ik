include (CheckIncludeFiles)
include (CheckCSourceCompiles)
include (CMakeDependentOption)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    set (IK_MEMORY_DEBUGGING_DEFAULT ON)
else ()
    set (IK_MEMORY_DEBUGGING_DEFAULT OFF)
endif ()

option (IK_BENCHMARKS "Whether to build benchmark tests or not (requires C++)" OFF)
option (IK_DOT_EXPORT "When enabled, the generated chains are dumped to DOT for debug purposes" OFF)
set (IK_LIB_TYPE "STATIC" CACHE STRING "SHARED or STATIC library")
option (IK_MEMORY_DEBUGGING "Global switch for memory options. Keeps track of the number of allocations and de-allocations and prints a report when the program shuts down" ${IK_MEMORY_DEBUGGING_DEFAULT})
cmake_dependent_option (IK_MEMORY_BACKTRACE "Generates backtraces for every malloc(), making it easy to track down memory leaks" ON "IK_MEMORY_DEBUGGING;NOT WIN32;NOT CYGWIN" OFF)
option (IK_PIC "Position independent code when building as a static library" ON)
set (IK_PRECISION "double" CACHE STRING "Type to use for real numbers")
option (IK_PROFILING "Compiles with -pg on linux" OFF)
option (IK_PYTHON "Compiles the library so it can also be loaded as a python module" OFF)
option (IK_TESTS "Whether to build unit tests or not (requires C++)" OFF)

string (REPLACE " " "_" IK_PRECISION_CAPS_AND_NO_SPACES ${IK_PRECISION})
string (TOUPPER ${IK_PRECISION_CAPS_AND_NO_SPACES} IK_PRECISION_CAPS_AND_NO_SPACES)

if (IK_BENCHMARKS OR IK_TESTS)
    set (CXX_LANGUAGE "CXX")
endif ()
project ("ik"
    VERSION 0.2.0
    LANGUAGES C ${CXX_LANGUAGE})

# Must use GNUInstallDirs to install libraries into correct locations on all
# platforms.
include (GNUInstallDirs)

# Determine visibility macros if the library is a shared library.
if (IK_LIB_TYPE MATCHES "SHARED")
    check_c_source_compiles ("__declspec(dllexport) void foo(void); int main(void) { return 0; }" DLLEXPORT_VISIBILITY)
    check_c_source_compiles ("__declspec(dllimport) void foo(void); int main(void) { return 0; }" DLLIMPORT_VISIBILITY)
    check_c_source_compiles ("__attribute__((visibility(\"default\"))) void foo(void); int main(void) { return 0; }" DEFAULT_VISIBILITY)
    check_c_source_compiles ("__attribute__((visibility(\"hidden\"))) void foo(void); int main(void) { return 0; }"  HIDDEN_VISIBILITY)
    if (DLLEXPORT_VISIBILITY AND DLLIMPORT_VISIBILITY)
        set (IK_HELPER_API_EXPORT "__declspec(dllexport)")
        set (IK_HELPER_API_IMPORT "__declspec(dllimport)")
        set (IK_HELPER_API_LOCAL "")
    elseif (DEFAULT_VISIBILITY AND HIDDEN_VISIBILITY)
        set (IK_HELPER_API_EXPORT "__attribute__((visibility(\"default\")))")
        set (IK_HELPER_API_IMPORT "__attribute__((visibility(\"default\")))")
        set (IK_HELPER_API_LOCAL  "__attribute__((visibility(\"hidden\")))")
    else ()
        message (FATAL_ERROR "Don't know how to define visibility macros for this compiler")
    endif ()
else ()
    set (IK_HELPER_API_EXPORT "")
    set (IK_HELPER_API_IMPORT "")
    set (IK_HELPER_API_LOCAL  "")
endif ()

# Check if we need to use pstdint.h or if stdint.h is available
check_include_files ("stdint.h" IK_HAVE_STDINT_H)

# Check if we can warn about unused function results
check_c_source_compiles ("int __attribute__((warn_unused_result)) f(int z) { return z*z + z*2 + z/3 + 23; } int main(void) { return f(4); }" HAVE_WARN_UNUSED)
check_c_source_compiles ("int _Check_return_ f(int z) { return z*z + z*2 + z/3 + 23; } int main(void) { return f(4); }" HAVE_CHECK_RETURN)
if (HAVE_WARN_UNUSED)
    set (IK_WARN_UNUSED "__attribute__((warn_unused_result))")
elseif (HAVE_CHECK_RETURN)
    set (IK_WARN_UNUSED "_Check_return_")
endif ()

set (IK_HOST_COMPUTER ${CMAKE_HOST_SYSTEM})
set (IK_COMPILER ${CMAKE_C_COMPILER_ID})
find_program (UNAME_PROGRAM uname)
if (UNAME_PROGRAM)
    execute_process (
        COMMAND ${UNAME_PROGRAM} -o -r -m -p -i
        OUTPUT_VARIABLE IK_HOST_COMPUTER)
    string (REPLACE "\n" "" IK_HOST_COMPUTER ${IK_HOST_COMPUTER})
else ()
    set (IK_HOST_COMPUTER ${CMAKE_HOST_SYSTEM_NAME})
endif ()

# Required for build info
find_package (Git)
if (GIT_FOUND)
    execute_process (COMMAND ${GIT_EXECUTABLE} describe --tags RESULT_VARIABLE RESULT OUTPUT_VARIABLE COMMIT_TAG)
    if (RESULT)
        set (COMMIT_TAG "(unknown tag)")
    else ()
        string (REPLACE "\n" "" COMMIT_TAG ${COMMIT_TAG})
    endif ()
    execute_process (COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD RESULT_VARIABLE RESULT OUTPUT_VARIABLE COMMIT_HASH)
    if (RESULT)
        set (COMMIT_HASH "(commit hash unknown)")
    else ()
        string (REPLACE "\n" "" COMMIT_HASH ${COMMIT_HASH})
    endif ()
    set (IK_COMMIT_INFO "${COMMIT_TAG} (${COMMIT_HASH})")
else ()
    set (IK_COMMIT_INFO "unknown (git not found!)")
    message (WARNING "Git not found. Build will not contain git revision info.")
endif ()

# Need pthread for unit tests
if (IK_TESTS)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif ()

configure_file ("templates/config.h.in"
                "include/public/ik/config.h")

set (IK_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
configure_file ("templates/build_info.c.in" "src/build_info.c")

###############################################################################
# All source file definitions
###############################################################################

# Update build info every time the project is compiled
set (GENERATED_BUILD_INFO_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/private/ik/build_info_for_every_compile.h")
add_custom_command (OUTPUT always_rebuild COMMAND cmake -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/nop.cmake COMMENT "")
add_custom_command (
    OUTPUT ${GENERATED_BUILD_INFO_HEADER}
    COMMAND ${CMAKE_COMMAND}
            -DOUTPUT_FILE=${GENERATED_BUILD_INFO_HEADER}
            -DIK_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -DIK_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_timestamp.cmake"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_timestamp.cmake" always_rebuild
    COMMENT "Updating build information"
    VERBATIM)

set (IK_HEADERS
    "include/private/ik/backtrace.h"
    "include/private/ik/chain.h"
    "include/private/ik/memory.h"
    "include/public/ik/bstv.h"
    "include/public/ik/build_info.h"
    "include/public/ik/constraint.h"
    "include/public/ik/effector.h"
    "include/public/ik/ik.h"
    "include/public/ik/log.h"
    "include/public/ik/node.h"
    "include/public/ik/pstdint.h"
    "include/public/ik/quat.h"
    "include/public/ik/retcodes.h"
    "include/public/ik/solver.h"
    "include/public/ik/tests.h"
    "include/public/ik/transform.h"
    "include/public/ik/util.h"
    "include/public/ik/vec3.h"
    "include/public/ik/vector.h"
    "templates/config.h.in"
    "${GENERATED_BUILD_INFO_HEADER}")
set (IK_SOURCES
    "src/bstv.c"
    "src/chain.c"
    "src/ik.c"
    "src/log_static.c"
    "src/memory.c"
    "src/quat.c"
    "src/retcodes.c"
    "src/transform_chains.c"
    "src/transform_tree.c"
    "src/util.c"
    "src/vec3.c"
    "src/vector.c"
    "src/solver/solver_static.c"
    "src/solver/base/constraint_base.c"
    "src/solver/base/effector_base.c"
    "src/solver/base/node_base.c"
    "src/solver/base/solver_base.c"
    "src/solver/FABRIK/node_FABRIK.c"
    "src/solver/FABRIK/solver_FABRIK.c"
    "src/solver/MSS/solver_MSS.c"
    "src/solver/ONE_BONE/solver_ONE_BONE.c"
    "src/solver/TWO_BONE/solver_TWO_BONE.c"
    "$<$<NOT:$<BOOL:${IK_TESTS}>>:src/tests_static_stub.c>"
    "${CMAKE_CURRENT_BINARY_DIR}/src/build_info.c"
    $<$<PLATFORM_ID:Linux>:
        "src/platform/linux/backtrace_linux.c"
    >
    "templates/build_info.c.in")
set (IK_VTABLES
    "include/vtables/build_info_static.v"
    "include/vtables/constraint_base.v"
    "include/vtables/effector_base.v"
    "include/vtables/log_static.v"
    "include/vtables/node_base.v"
    "include/vtables/solver_base.v"
    "include/vtables/solver_FABRIK.v"
    "include/vtables/solver_MSS.v"
    "include/vtables/solver_ONE_BONE.v"
    "include/vtables/solver_static.v"
    "include/vtables/solver_TWO_BONE.v"
    "include/vtables/tests_static.v")
set (IK_PYTHON_HEADERS
    "include/python/ik/python/ik_module_info.h"
    "include/python/ik/python/ik_module_log.h"
    "include/python/ik/python/ik_module_node.h"
    "include/python/ik/python/ik_module_quat.h"
    "include/python/ik/python/ik_module_solver.h"
    "include/python/ik/python/ik_module_vec3.h")
set (IK_PYTHON_SOURCES
    "src/python/ik_module.c"
    "src/python/ik_module_info.c"
    "src/python/ik_module_log.c"
    "src/python/ik_module_node.c"
    "src/python/ik_module_quat.c"
    "src/python/ik_module_solver.c"
    "src/python/ik_module_vec3.c")
set (IK_TESTS_SOURCES
    "thirdparty/googletest/src/gtest-all.cc"
    "thirdparty/googlemock/src/gmock-all.cc"
    "src/tests/environment_library_init.cpp"
    "src/tests/tests_static.cpp"
    "src/tests/test_bstv.cpp"
    "src/tests/test_effector.cpp"
    "src/tests/test_FABRIK.cpp"
    "src/tests/test_node.cpp"
    "src/tests/test_quat.cpp"
    "src/tests/test_transform_chain.cpp"
    "src/tests/test_transform_tree.cpp"
    "src/tests/test_vector.cpp"
    "src/tests/test_vec3.cpp"
    $<$<BOOL:${IK_PYTHON}>:${CMAKE_CURRENT_BINARY_DIR}/src/test_python_bindings.cpp>)

# IK preprocessor script
set (Python_ADDITIONAL_VERSIONS 3)
find_package (PythonInterp REQUIRED)
set (IK_VTABLES_H "")
set (PYTHON_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/ik_gen_vtable.py)
macro (add_vtable_file VTABLE_FILE)
    get_filename_component (H_FILE_BASENAME ${VTABLE_FILE} NAME)
    string (REPLACE ".v" ".h" H_FILE_BASENAME ${H_FILE_BASENAME})
    set (H_FILE ${CMAKE_CURRENT_BINARY_DIR}/include/private/ik/${H_FILE_BASENAME})
    add_custom_command (
        OUTPUT ${H_FILE}
        DEPENDS ${VTABLE_FILE} ${PYTHON_SCRIPT}
        COMMAND ${PYTHON_EXECUTABLE} ${PYTHON_SCRIPT} ${VTABLE_FILE} ${H_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating vtable ${H_FILE}" VERBATIM)
    set (IK_VTABLES_H ${IK_VTABLES_H} ${H_FILE})
endmacro ()
foreach (VTABLE_FILE ${IK_VTABLES})
    add_vtable_file (${VTABLE_FILE})
endforeach ()

###############################################################################
# Main library
###############################################################################

add_library (ik_obj OBJECT
    ${IK_HEADERS}
    ${IK_SOURCES}
    ${IK_VTABLES}
    ${IK_VTABLES_H})
set_target_properties (ik_obj
    PROPERTIES
        C_STANDARD 11
        POSITION_INDEPENDENT_CODE ${IK_PIC})
target_include_directories (ik_obj
    PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/include/public
            ${CMAKE_CURRENT_BINARY_DIR}/include/private
            ${CMAKE_CURRENT_SOURCE_DIR}/include/public
            ${CMAKE_CURRENT_SOURCE_DIR}/include/private)
target_compile_definitions (ik_obj
    PRIVATE IK_BUILDING
    PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_options (ik_obj
    PRIVATE $<$<C_COMPILER_ID:MSVC>:
        /EHa /MTd /W4 /WX /wd4305 /wd4201 /wd4706 /wd4100 /wd4244 /wd4477 /D_CRT_SECURE_NO_DEPRECATE
    >
    PRIVATE $<$<C_COMPILER_ID:GNU>:
        -W -Wall -Wextra -Werror -Wshadow -Wconversion -Wno-unused-parameter
        -Wno-conversion -Wno-implicit-fallthrough -pedantic -pedantic-errors -fno-strict-aliasing -ffast-math
    >
    PRIVATE $<$<C_COMPILER_ID:Clang>:
        -W -Wall -Wextra -Werror -Wshadow -Wconversion -Wno-unused-parameter
        -Wno-implicit-fallthrough -Wno-conversion -pedantic -pedantic-errors -fno-strict-aliasing -ffast-math
    >
)

###############################################################################
# Python bindings
###############################################################################

if (IK_PYTHON)
    find_package (PythonLibs 3 REQUIRED)
    add_library (ik_python_obj OBJECT
        ${IK_PYTHON_SOURCES}
        ${IK_PYTHON_HEADERS})
    set_target_properties (ik_python_obj
        PROPERTIES POSITION_INDEPENDENT_CODE ${IK_PIC})
    target_include_directories (ik_python_obj
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/include/public
            ${CMAKE_CURRENT_SOURCE_DIR}/include/public
            ${CMAKE_CURRENT_SOURCE_DIR}/include/python
            ${PYTHON_INCLUDE_DIRS})
    target_compile_options (ik_python_obj
        PUBLIC $<$<C_COMPILER_ID:MSVC>:
            /EHa /MTd /W4 /WX /wd4115 /wd4201 /wd4100 /D_CRT_SECURE_NO_DEPRECATE
        >
        PUBLIC $<$<C_COMPILER_ID:GNU>:
            -Wall -Wextra -Werror -pedantic -pedantic-errors -Wno-missing-field-initializers -Wshadow
        >
        PUBLIC $<$<C_COMPILER_ID:Clang>:
            -Wall -Wextra -Werror -pedantic -pedantic-errors -Wno-missing-field-initializers -Wshadow
        >
    )
    target_compile_definitions (ik_python_obj
        PRIVATE IK_BUILDING  # The library is being built
        PRIVATE $<$<CONFIG:Debug>:
            DEBUG
        >
    )
endif ()

###############################################################################
# Unit tests
###############################################################################

if (IK_TESTS)
    if (IK_PYTHON)
        set (GENERATE_PYTHON_TESTS_CMD ${CMAKE_COMMAND}
            -DIK_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -DIK_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_python_unit_tests.cmake")
        add_custom_command (OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/src/test_python_bindings.cpp"
            COMMAND ${GENERATE_PYTHON_TESTS_CMD}
            DEPENDS
                "cmake/generate_python_unit_tests.cmake"
                "templates/test_python_bindings.cpp.in"
                ${PYTHON_TEST_SOURCE_FILES})
        add_custom_target (ik_generate_unit_tests ALL
            DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/src/test_python_bindings.cpp")
    endif ()
    add_library (ik_tests_obj OBJECT
        ${IK_TESTS_SOURCES})
    if (IK_PYTHON)
        add_dependencies (ik_tests_obj ik_generate_unit_tests)
    endif ()
    target_include_directories (ik_tests_obj
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/include/public
            ${CMAKE_CURRENT_BINARY_DIR}/include/private
            ${CMAKE_CURRENT_SOURCE_DIR}/include/public
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/googlemock
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/googlemock/include
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/googletest
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/googletest/include
            $<$<BOOL:${IK_PYTHON}>:${PYTHON_INCLUDE_DIRS}>)
    set_target_properties (ik_tests_obj
        PROPERTIES POSITION_INDEPENDENT_CODE ${IK_PIC})
    target_compile_options (ik_tests_obj
        PRIVATE $<$<C_COMPILER_ID:MSVC>:
            /MTd /D_CRT_SECURE_NO_DEPRECATE
        >
        PRIVATE $<$<C_COMPILER_ID:GNU>:
            -Wno-unused-result
        >
        PRIVATE $<$<C_COMPILER_ID:Clang>:
            -Wno-unused-result
        >
    )
    target_compile_definitions (ik_tests_obj
        PRIVATE IK_BUILDING  # The library is being built
        PRIVATE $<$<CONFIG:Debug>:
            DEBUG
        >
    )
endif ()

###############################################################################
# The actual library
###############################################################################

add_library (ik ${IK_LIB_TYPE}
    $<TARGET_OBJECTS:ik_obj>
    $<$<BOOL:${IK_TESTS}>:$<TARGET_OBJECTS:ik_tests_obj>>
    $<$<BOOL:${IK_PYTHON}>:$<TARGET_OBJECTS:ik_python_obj>>)

target_include_directories (ik
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/public>
        $<INSTALL_INTERFACE:include>)

set_target_properties (ik PROPERTIES
    PREFIX ""
    DEBUG_POSTFIX ""
    SOVERSION ${PROJECT_VERSION})

if (IK_PYTHON)
    target_link_libraries (ik PRIVATE ${PYTHON_LIBRARIES})
endif ()

if (IK_TESTS)
    target_link_libraries (ik PRIVATE Threads::Threads)
endif ()

if (IK_TESTS)
    add_executable (ik_tests "src/tests/run_tests.c")
    target_link_libraries (ik_tests PUBLIC ik)
    set_target_properties (ik_tests PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
    install (TARGETS ik_tests
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

###############################################################################
# Dependency settings
###############################################################################

message (STATUS "------------------------------------------------------------")
message (STATUS "IK settings")
message (STATUS " + Configuration: ${CMAKE_BUILD_TYPE}")
MESSAGE (STATUS " + Benchmarks: ${IK_BENCHMARKS}")
MESSAGE (STATUS " + DOT Export: ${IK_DOT_EXPORT}")
message (STATUS " + Library type: ${IK_LIB_TYPE}")
message (STATUS " + Memory debugging: ${IK_MEMORY_DEBUGGING}")
message (STATUS " + Memory backtraces: ${IK_MEMORY_BACKTRACE}")
message (STATUS " + PIC (Position independent code): ${IK_PIC}")
message (STATUS " + Precision: ${IK_PRECISION}")
message (STATUS " + Python bindings: ${IK_PYTHON}")
message (STATUS " + Profiling: ${IK_PROFILING}")
message (STATUS " + Unit Tests: ${IK_TESTS}")
message (STATUS "------------------------------------------------------------")

###############################################################################
# install targets
###############################################################################

install (
    TARGETS ik
    EXPORT IKConfig
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install (DIRECTORY "include/public/ik" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/public/ik" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (EXPORT IKConfig DESTINATION "share/ik/cmake")
export (TARGETS ik FILE IKConfig.cmake)
