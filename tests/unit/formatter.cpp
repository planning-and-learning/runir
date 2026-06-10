#include <gtest/gtest.h>
#include <runir/formatter.hpp>

TEST(RunirFormatterTest, ToStringUsesFmtFormatter) { EXPECT_EQ(runir::to_string(42), "42"); }
