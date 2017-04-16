project (iktests)

add_subdirectory ("googlemock")

file (GLOB tests_SOURCES "src/*.cpp")
add_executable (iktests ${tests_SOURCES})
include_directories ("googletest/include")
include_directories ("googlemock/include")
include_directories ("${CMAKE_SOURCE_DIR}/ik/include")
include_directories ("${CMAKE_BINARY_DIR}/ik/include")
target_link_libraries (iktests ik)
target_link_libraries (iktests gmock)
target_link_libraries (iktests gmock_main)

install (
    TARGETS iktests
    RUNTIME DESTINATION "bin"
)

