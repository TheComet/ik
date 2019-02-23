include (CheckIncludeFiles)
include (CheckCSourceCompiles)
include (CMakeDependentOption)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    set (DEBUG_FEATURE ON)
    set (RELEASE_FEATURE OFF)
else ()
    set (DEBUG_FEATURE OFF)
    set (RELEASE_FEATURE ON)
endif ()

set (IK_LIB_TYPE "SHARED" CACHE STRING "SHARED or STATIC library")
set (IK_API_NAME "ik" CACHE STRING "This is the name of the symbol that is exported. Also determines the name of the library.")
option (IK_BENCHMARKS "Whether to build benchmark tests or not (requires C++)" OFF)
option (IK_BUILDINFO "If true (default) the updated build number is written to a header, which causes the library to recompile info.c. This can get annoying for developers, so disable this if you are annoyed." ${RELEASE_FEATURE})
option (IK_DOT_EXPORT "When enabled, the generated chains are dumped to DOT for debug purposes" OFF)
option (IK_MEMORY_DEBUGGING "Global switch for memory options. Keeps track of the number of allocations and de-allocations and prints a report when the program shuts down" ${DEBUG_FEATURE})
cmake_dependent_option (IK_MEMORY_BACKTRACE "Generates backtraces for every malloc(), making it easy to track down memory leaks" ON "IK_MEMORY_DEBUGGING;NOT WIN32;NOT CYGWIN" OFF)
option (IK_HASHMAP_STATS "Record statistics on the use of hashmaps" OFF)
option (IK_PIC "Position independent code when building as a static library" ON)
set (IK_PRECISION "double" CACHE STRING "Type to use for real numbers")
option (IK_PROFILING "Compiles with -pg on linux" OFF)
option (IK_PYTHON "Compiles the library so it can also be loaded as a python module" OFF)
set (IK_PYTHON_VERSION 3 CACHE STRING "The version of python to use if IK_PYTHON=ON")
option (IK_TESTS "Whether to build unit tests or not (requires C++)" OFF)
option (IK_VECTOR_64BIT "Allows vector_t to contain more than 2^32 elements, but makes the structure 32 bytes instead of 20 bytes" OFF)

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
configure_file ("templates/info.c.in" "src/info.c")

###############################################################################
# All source file definitions
###############################################################################

# Update build info every time the project is compiled
set (UPDATE_INFO_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/private/ik/info_for_every_compile.h")
add_custom_target (ik_ifec_header
    COMMAND ${CMAKE_COMMAND}
            -DOUTPUT_FILE=${UPDATE_INFO_HEADER}
            -DIK_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -DIK_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DIK_BUILDINFO=${IK_BUILDINFO}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_timestamp.cmake"
    COMMENT "Updating build information"
    VERBATIM)
add_custom_command (OUTPUT ${UPDATE_INFO_HEADER} COMMAND cmake -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/nop.cmake DEPENDS ik_ifec_header)

set (IK_HEADERS
    "include/private/ik/backtrace.h"
    "include/private/ik/chain.h"
    "include/private/ik/memory.h"
    "include/public/ik/btree.h"
    "include/public/ik/callbacks.h"
    "include/public/ik/constraint.h"
    "include/public/ik/effector.h"
    "include/public/ik/hash.h"
    "include/public/ik/hashmap.h"
    "include/public/ik/ik.h"
    "include/public/ik/info.h"
    "include/public/ik/init.h"
    "include/public/ik/log.h"
    "include/public/ik/mat3x3.h"
    "include/public/ik/node.h"
    "include/private/ik/ntf.h"
    "include/private/ik/node_data.h"
    "include/public/ik/pole.h"
    "include/public/ik/pstdint.h"
    "include/public/ik/quat.h"
    "include/public/ik/refcount.h"
    "include/public/ik/retcodes.h"
    "include/public/ik/solver.h"
    "include/private/ik/solver_head.h"
    "include/public/ik/tests.h"
    "include/public/ik/transform.h"
    "include/public/ik/util.h"
    "include/public/ik/vec3.h"
    "include/public/ik/vector.h"
    "templates/config.h.in"
    "${UPDATE_INFO_HEADER}")
set (IK_SOURCES
    "src/btree.c"
    #"src/chain.c"
    "src/callbacks.c"
    "src/constraint.c"
    "src/effector.c"
    "src/hash.c"
    "src/hashmap.c"
    "src/ik.c"
    "src/init.c"
    "src/log.c"
    "src/mat3x3.c"
    "src/memory.c"
    "src/node.c"
    "src/node_data.c"
    "src/ntf.c"
    #"src/pole.c"
    "src/quat.c"
    "src/refcount.c"
    "src/retcodes.c"
    #"src/solver.c"
    #"src/solver_FABRIK.c"
    #"src/solver_MSS.c"
    #"src/solver_ONE_BONE.c"
    #"src/solver_TWO_BONE.c"
    #"src/transform_chains.c"
    #"src/transform_node.c"
    #"src/util.c"
    "src/vec3.c"
    "src/vector.c"
    "$<$<NOT:$<BOOL:${IK_TESTS}>>:src/tests/tests_run_stub.c>"
    "${CMAKE_CURRENT_BINARY_DIR}/src/info.c"
    $<$<PLATFORM_ID:Linux>:
        "src/platform/linux/backtrace_linux.c"
    >
    "templates/info.c.in")
set (IK_PYTHON_HEADERS
    "include/python/ik/python/ik_module_info.h"
    "include/python/ik/python/ik_module_log.h"
    "include/python/ik/python/ik_type_Constraint.h"
    "include/python/ik/python/ik_type_Node.h"
    "include/python/ik/python/ik_type_Pole.h"
    "include/python/ik/python/ik_type_Quat.h"
    "include/python/ik/python/ik_type_Solver.h"
    "include/python/ik/python/ik_type_Vec3.h")
set (IK_PYTHON_SOURCES
    "src/python/ik_module.c"
    "src/python/ik_module_info.c"
    "src/python/ik_module_log.c"
    "src/python/ik_type_Constraint.c"
    "src/python/ik_type_Effector.c"
    "src/python/ik_type_Node.c"
    "src/python/ik_type_Pole.c"
    "src/python/ik_type_Quat.c"
    "src/python/ik_type_Solver.c"
    "src/python/ik_type_Vec3.c")
set (IK_TESTS_SOURCES
    "thirdparty/googletest/src/gtest-all.cc"
    "thirdparty/googlemock/src/gmock-all.cc"
    "src/tests/environment_library_init.cpp"
    "src/tests/tests_run.cpp"
    "src/tests/test_btree.cpp"
    "src/tests/test_constraint.cpp"
    #"src/tests/test_effector.cpp"
   # "src/tests/test_FABRIK.cpp"
    "src/tests/test_hashmap.cpp"
    "src/tests/test_mat3x3.cpp"
    "src/tests/test_node.cpp"
    "src/tests/test_ntf.cpp"
    "src/tests/test_pole.cpp"
    "src/tests/test_quat.cpp"
   # "src/tests/test_transform_chain.cpp"
   # "src/tests/test_transform_chain_rotations.cpp"
   # "src/tests/test_transform_chain_translations.cpp"
   # "src/tests/test_transform_node.cpp"
   # "src/tests/test_transform_node_rotations.cpp"
   # "src/tests/test_transform_node_translations.cpp"
   # "src/tests/test_TWO_BONE.cpp"
    "src/tests/test_vector.cpp"
    "src/tests/test_vec3.cpp"
    $<$<BOOL:${IK_PYTHON}>:${CMAKE_CURRENT_BINARY_DIR}/src/test_python_bindings.cpp>)
set (IK_BENCHMARK_SOURCES
    "src/benchmarks/bench_FABRIK_solver.cpp"
    "src/benchmarks/bench_solve.cpp")

###############################################################################
# Main library
###############################################################################

add_library (ik_obj OBJECT
    ${IK_HEADERS}
    ${IK_SOURCES})
set_target_properties (ik_obj
    PROPERTIES
        C_STANDARD 90
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
        /EHa /MTd /W4 /WX /wd4305 /wd4201 /wd4706 /wd4100 /wd4244 /wd4477 /wd4003 /D_CRT_SECURE_NO_DEPRECATE
    >
    PRIVATE $<$<C_COMPILER_ID:GNU>:
        -W -Wall -Wextra -Werror -Wshadow -Wconversion -Wno-unused-parameter -Wno-conversion -Wno-implicit-fallthrough -Wno-overlength-strings
        -pedantic -pedantic-errors -fno-strict-aliasing -ffast-math -flto
        $<$<BOOL:IK_PROFILING>:-pg -fno-omit-frame-pointer>
    >
    PRIVATE $<$<C_COMPILER_ID:Clang>:
        -W -Wall -Wextra -Werror -Wshadow -Wconversion -Wno-unused-parameter -Wno-conversion -Wno-implicit-fallthrough
        -pedantic -pedantic-errors -fno-strict-aliasing -ffast-math
        $<$<BOOL:IK_PROFILING>:-pg -fno-omit-frame-pointer>
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
        PROPERTIES
            C_STANDARD 90
            POSITION_INDEPENDENT_CODE ${IK_PIC})
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
            -Wall -Wextra -Werror -pedantic -pedantic-errors
            -Wno-missing-field-initializers -Wshadow -flto
            $<$<BOOL:IK_PROFILING>:-pg -fno-omit-frame-pointer>
        >
        PUBLIC $<$<C_COMPILER_ID:Clang>:
            -Wall -Wextra -Werror -pedantic -pedantic-errors -Wno-missing-field-initializers -Wshadow
            $<$<BOOL:IK_PROFILING>:-pg -fno-omit-frame-pointer>
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
            ${CMAKE_CURRENT_SOURCE_DIR}/include/private
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
            $<$<BOOL:IK_PROFILING>:-pg -fno-omit-frame-pointer>
        >
        PRIVATE $<$<C_COMPILER_ID:Clang>:
            -Wno-unused-result
            $<$<BOOL:IK_PROFILING>:-pg -fno-omit-frame-pointer>
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
# Benchmarks
###############################################################################

if (IK_BENCHMARKS)
    add_subdirectory ("thirdparty/benchmark")
    set_target_properties (benchmark
    PROPERTIES
        POSITION_INDEPENDENT_CODE ${IK_PIC})
    add_library (ik_benchmarks_obj OBJECT
        ${IK_BENCHMARK_SOURCES})
    target_include_directories (ik_benchmarks_obj
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/include/public
            ${CMAKE_CURRENT_BINARY_DIR}/include/private
            ${CMAKE_CURRENT_SOURCE_DIR}/include/public
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/benchmark/include
            $<$<BOOL:${IK_PYTHON}>:${PYTHON_INCLUDE_DIRS}>)
    set_target_properties (ik_benchmarks_obj
        PROPERTIES POSITION_INDEPENDENT_CODE ${IK_PIC})
    target_compile_options (ik_benchmarks_obj
        PRIVATE $<$<C_COMPILER_ID:MSVC>:
            /MTd /D_CRT_SECURE_NO_DEPRECATE
        >
        PRIVATE $<$<C_COMPILER_ID:GNU>:
            -Wno-unused-result -flto
            $<$<BOOL:IK_PROFILING>:-pg -fno-omit-frame-pointer>
        >
        PRIVATE $<$<C_COMPILER_ID:Clang>:
            -Wno-unused-result
            $<$<BOOL:IK_PROFILING>:-pg -fno-omit-frame-pointer>
        >
    )
    target_compile_definitions (ik_benchmarks_obj
        PRIVATE IK_BUILDING  # The library is being built
        PRIVATE MS_NO_COREDLL # Stops pyconfig.h from trying to autolink python libraries; cmake handles which libraries get linked
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
    $<$<BOOL:${IK_BENCHMARKS}>:$<TARGET_OBJECTS:ik_benchmarks_obj>>
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
    OUTPUT_NAME ${IK_API_NAME}
    SOVERSION ${PROJECT_VERSION})

if (UNIX)
    target_link_libraries (ik PRIVATE m)
endif (UNIX)

if (IK_PYTHON)
    target_link_libraries (ik PRIVATE ${PYTHON_LIBRARIES})
endif ()

if (IK_TESTS)
    target_link_libraries (ik PRIVATE Threads::Threads)
    add_executable (ik_tests "src/tests/tests_main.c")
    target_link_libraries (ik_tests PUBLIC ik)
    set_target_properties (ik_tests PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
    install (TARGETS ik_tests
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if (IK_BENCHMARKS)
    add_executable (ik_benchmarks "src/benchmarks/run_benchmarks.cpp")
    target_link_libraries (ik_benchmarks PUBLIC ik)
    target_link_libraries (ik PRIVATE benchmark)
    target_include_directories (ik_benchmarks
            PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/benchmark/include)
    set_target_properties (ik_benchmarks PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
    install (TARGETS ik_benchmarks
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

###############################################################################
# Dependency settings
###############################################################################

message (STATUS "------------------------------------------------------------")
message (STATUS "IK settings")
message (STATUS " + Configuration: ${CMAKE_BUILD_TYPE}")
message (STATUS " + Library type: ${IK_LIB_TYPE}")
message (STATUS " + API Name: ${IK_API_NAME}")
message (STATUS " + Benchmarks: ${IK_BENCHMARKS}")
message (STATUS " + Buildinfo: ${IK_BUILDINFO}")
message (STATUS " + DOT Export: ${IK_DOT_EXPORT}")
message (STATUS " + Memory debugging: ${IK_MEMORY_DEBUGGING}")
message (STATUS " + Memory backtraces: ${IK_MEMORY_BACKTRACE}")
message (STATUS " + Hashmap statistics: ${IK_HASHMAP_STATS}")
message (STATUS " + PIC (Position independent code): ${IK_PIC}")
message (STATUS " + Precision: ${IK_PRECISION}")
message (STATUS " + Profiling: ${IK_PROFILING}")
message (STATUS " + Python bindings: ${IK_PYTHON}")
message (STATUS " + Python version: ${IK_PYTHON_VERSION}")
message (STATUS " + Unit Tests: ${IK_TESTS}")
message (STATUS " + 64-bit vector: ${IK_VECTOR_64BIT}")
message (STATUS "------------------------------------------------------------")

###############################################################################
# install targets
###############################################################################

install (
    TARGETS ik
    EXPORT ikConfig
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install (DIRECTORY "include/public/ik" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/public/ik" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (EXPORT ikConfig DESTINATION "share/ik/cmake")
export (TARGETS ik FILE ikConfig.cmake)
export (PACKAGE ik)
