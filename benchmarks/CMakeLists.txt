project (iktests)

add_subdirectory ("benchmark")

file (GLOB bench_SOURCES "src/*.cpp")
add_executable (ikbenchmarks ${bench_SOURCES})
include_directories ("benchmark/include")
include_directories ("${CMAKE_SOURCE_DIR}/ik/include")
include_directories ("${CMAKE_BINARY_DIR}/ik/include")
target_link_libraries (ikbenchmarks ik)
target_link_libraries (ikbenchmarks benchmark)

install (
    TARGETS ikbenchmarks
    RUNTIME DESTINATION "bin"
)

