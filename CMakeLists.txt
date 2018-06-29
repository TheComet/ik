cmake_minimum_required (VERSION 3.2 FATAL_ERROR)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

project ("Inverse Kinematics" LANGUAGES)

# Must use GNUInstallDirs to install libraries into correct locations on all
# platforms.
include(GNUInstallDirs)

add_subdirectory ("ik")
