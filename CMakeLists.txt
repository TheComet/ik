cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

project ("Inverse Kinematics" LANGUAGES C)

# Unit testing library
if (IK_TESTS OR CSTRUCTURES_TESTS)
    add_subdirectory ("thirdparty/googletest")
    set_property (TARGET gmock PROPERTY POSITION_INDEPENDENT_CODE ${IK_PIC})
    set_property (TARGET gtest PROPERTY POSITION_INDEPENDENT_CODE ${IK_PIC})
endif ()

# Benchmarking library
if (IK_BENCHMARKS OR CSTRUCTURES_BENCHMARKS)
    add_subdirectory ("thirdparty/benchmark")
    set_target_properties (benchmark PROPERTIES POSITION_INDEPENDENT_CODE ${IK_PIC})
endif ()

add_subdirectory ("thirdparty/cstructures/cstructures")
add_subdirectory ("ik")
