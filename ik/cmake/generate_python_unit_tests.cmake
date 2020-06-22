execute_process (COMMAND ${CMAKE_COMMAND} -E copy
    "${IK_SOURCE_DIR}/templates/test_python_bindings.cpp.in"
    "${IK_BINARY_DIR}/src/test_python_bindings.cpp")

separate_arguments (IK_PYTHON_TESTS_SOURCES)

foreach (f ${IK_PYTHON_TESTS_SOURCES})
    file (READ ${IK_SOURCE_DIR}/${f} TEST_SRC)
    string (CONCAT TEST_SRC ${TEST_SRC} "\n"
        "test_results = unittest.main(exit=False).result\n"
        "if len(test_results.errors) > 0 or len(test_results.failures) > 0:\n"
        "    raise RuntimeError(\"(C++ Harness): Python unittests failed\")")
    string (REPLACE "\"" "\\\"" TEST_SRC ${TEST_SRC})
    string (REPLACE "\n" "\\n\"\n\"" TEST_SRC ${TEST_SRC})
    get_filename_component (TEST_NAME ${f} NAME_WE)
    string (CONCAT TEST_SRC
        "TEST(NAME, ${TEST_NAME})\n"
        "{\n"
        "    int pythonRunResult = PyRun_SimpleString(\"${TEST_SRC}\")\;\n"
        "    EXPECT_THAT(pythonRunResult, Eq(0))\;\n"
        "}\n")
    message ("Adding test ${f}")
    file (APPEND "${IK_BINARY_DIR}/src/test_python_bindings.cpp" ${TEST_SRC})
endforeach ()
