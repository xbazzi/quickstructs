// clang-format on
#include "quick/structs/Vector.hh"

#include <gtest/gtest.h>
#include <iostream>
#include <string>
// clang-format off

class VectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup code if needed
    }
};

TEST_F(VectorTest, DefaultConstructor)
{
    quick::structs::Vector<int> vec;
    EXPECT_EQ(vec.size(), 0);
}

TEST_F(VectorTest, SizeConstructor)
{
    quick::structs::Vector<int> vec(5);
    EXPECT_EQ(vec.size(), 5);
}

TEST_F(VectorTest, PushBackIntegers)
{
    quick::structs::Vector<int> vec;
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 10);
    EXPECT_EQ(vec[1], 20);
    EXPECT_EQ(vec[2], 30);
}

TEST_F(VectorTest, PushBackStrings)
{
    quick::structs::Vector<std::string> vec;
    
    vec.push_back("Hello");
    vec.push_back("World");
    vec.push_back("Test");
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], "Hello");
    EXPECT_EQ(vec[1], "World");
    EXPECT_EQ(vec[2], "Test");
}

TEST_F(VectorTest, CapacityGrowth)
{
    quick::structs::Vector<int> vec;
    
    // Push enough elements to trigger capacity growth
    for (int i = 0; i < 20; ++i)
    {
        vec.push_back(i);
    }
    
    EXPECT_EQ(vec.size(), 20);
    for (int i = 0; i < 20; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}