#include "gmock/gmock.h"
#include "ik/log.h"
#include "ik/init.h"

using namespace testing;

class LibraryInitEnvironment : public testing::Environment
{
public:
    virtual ~LibraryInitEnvironment() {}

    virtual void SetUp()
    {
        testing::FLAGS_gtest_death_test_style = "threadsafe";
        ASSERT_THAT(ik_init(), Eq(IK_OK));
        ASSERT_THAT(ik_log_init(), Eq(IK_OK));
    }

    virtual void TearDown()
    {
        ik_log_deinit();
        EXPECT_THAT(ik_deinit(), Eq(0u)) << "Number of memory leaks";
    }
};

const testing::Environment* const libraryInitEnvironment =
        testing::AddGlobalTestEnvironment(new LibraryInitEnvironment);
