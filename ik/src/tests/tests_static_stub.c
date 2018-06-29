#include "ik/tests_static.h"
#include "ik/ik.h"
#include <stdio.h>

/* ------------------------------------------------------------------------- */
ikret_t
ik_tests_static_run(void)
{
    ik.log.message("Error: The IK library was built without unit tests. Recompile with -DIK_TESTS=ON if you want this functionality.");
    return IK_BUILT_WITHOUT_TESTS;
}
