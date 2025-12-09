// clang-format off

#include <gtest/gtest.h>

#include <algorithm>
#include <iostream>
#include <ranges>

#include "quick/utils/Timer.hpp" 
#include "quick/structs/SPSCQueue.hpp"
// clang-format on

class ThreadPoolTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  void TearDown() override {}

  static inline quick::structs::SPSCQueue<int, 1024UL> p_test_obj;
};
