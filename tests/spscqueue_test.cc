// clang-format off

#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>
#include <ranges>

#include "quick/utils/Timer.hpp" 
#include "test_utils.hh" 
#include "quick/structs/SPSCQueue.hpp"
// clang-format on

class SPSCQueueTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}

  quick::structs::SPSCQueue<int, 1024UL> p_test_obj;
};

TEST_F(SPSCQueueTest, AddElements) {
  quick::utils::Timer spsc_timer{"SPSCQueueTest"};
  EXPECT_EQ(p_test_obj.capacity(), 1024) << "Yup";
  EXPECT_EQ(p_test_obj.size(), 0) << "Yup";

  std::ranges::for_each(std::views::iota(0, 1022),
                        [this](int i) { EXPECT_TRUE(p_test_obj.push(i)); });

  EXPECT_EQ(p_test_obj.size(), 1022);
  EXPECT_TRUE(p_test_obj.push(1));
  EXPECT_TRUE(p_test_obj.push(2));
  EXPECT_EQ(p_test_obj.size(), 1024);

  // Push into full queue
  EXPECT_FALSE(p_test_obj.push(3));

  // Size shouldn't change
  EXPECT_EQ(p_test_obj.size(), 1024);
}