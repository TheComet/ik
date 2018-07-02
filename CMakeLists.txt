cmake_minimum_required (VERSION 3.2 FATAL_ERROR)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

project ("Inverse Kinematics" LANGUAGES)

add_subdirectory ("ik")
