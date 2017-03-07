project (app)

add_executable (app main.cpp)
target_link_libraries (app ik)
include_directories ("${CMAKE_SOURCE_DIR}/ik/include")
include_directories ("${CMAKE_BINARY_DIR}/ik/include")
