execute_process (COMMAND ${CMAKE_COMMAND} -E copy
    "${IK_SOURCE_DIR}/templates/test_python_bindings.cpp.in"
    "${IK_BINARY_DIR}/src/test_python_bindings.cpp")

file (GLOB PYTHON_TEST_SOURCE_FILES "${IK_SOURCE_DIR}/src/tests/python/*.py")

foreach (f ${PYTHON_TEST_SOURCE_FILES})
    file (READ ${f} TEST_SRC)
    string (CONCAT TEST_SRC ${TEST_SRC} "\nif len(unittest.main(exit=False).result.failures):\n    raise RuntimeError(\"Tests failed\")")
    string (REPLACE "\"" "\\\"" TEST_SRC ${TEST_SRC})
    string (REPLACE "\n" "\\n\"\n\"" TEST_SRC ${TEST_SRC})
    get_filename_component (TEST_NAME ${f} NAME_WE)
    string (CONCAT TEST_SRC
        "TEST_F(NAME, ${TEST_NAME})\n{\n"
        "    int result = PyRun_SimpleString(\""
        ${TEST_SRC}
        "    \")\;\n"
        "    EXPECT_THAT(result, Eq(0))\;\n"
        "}\n")
    message ("Adding test ${f}")
    file (APPEND "${IK_BINARY_DIR}/src/test_python_bindings.cpp" ${TEST_SRC})
endforeach ()
