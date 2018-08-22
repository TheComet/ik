#include "ik/tests.h"
#include <gmock/gmock.h>
#include <stdio.h>

ikret_t
ik_tests_run(int* argc, char** argv)
{

    // Since Google Mock depends on Google Test, InitGoogleMock() is
    // also responsible for initializing Google Test.  Therefore there's
    // no need for calling testing::InitGoogleTest() separately.
    testing::InitGoogleMock(argc, argv);
    if (RUN_ALL_TESTS() != 0)
        return IK_UNIT_TESTS_FAILED;
    return IK_OK;
}
