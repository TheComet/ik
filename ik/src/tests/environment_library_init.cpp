#include "gmock/gmock.h"
#include "ik/ik.h"

using namespace testing;

class LibraryInitEnvironment : public testing::Environment
{
public:
    virtual ~LibraryInitEnvironment() {}

    virtual void SetUp()
    {
        testing::FLAGS_gtest_death_test_style = "threadsafe";
        ASSERT_THAT(IK.init(), Eq(IK_OK));
    }

    virtual void TearDown()
    {
        EXPECT_THAT(IK.deinit(), Eq(0u)) << "Number of memory leaks";
    }
};

const testing::Environment* const libraryInitEnvironment =
        testing::AddGlobalTestEnvironment(new LibraryInitEnvironment);
