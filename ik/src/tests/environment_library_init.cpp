#include "gmock/gmock.h"
#include "ik/log.h"
#include "ik/init.h"

using namespace testing;

class LibraryInitEnvironment : public testing::Environment
{
public:
    virtual void SetUp() override
    {
        testing::FLAGS_gtest_death_test_style = "threadsafe";
        ASSERT_THAT(ik_init(), Eq(0));
        ASSERT_THAT(ik_log_init(), Eq(0));
    }

    virtual void TearDown() override
    {
        ik_log_deinit();
        EXPECT_THAT(ik_deinit(), Eq(0u)) << "Number of memory leaks";
    }
};

const testing::Environment* const libraryInitEnvironment =
        testing::AddGlobalTestEnvironment(new LibraryInitEnvironment);
