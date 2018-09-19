#include "ik/log.h"
#include "ik/tests.h"
#include <stdio.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_tests_run(int* argc, char** argv)
{
    ik_log_error("The IK library was built without unit tests. Recompile with -DIK_TESTS=ON if you want this functionality.");
    return IK_ERR_BUILT_WITHOUT_TESTS;
}
