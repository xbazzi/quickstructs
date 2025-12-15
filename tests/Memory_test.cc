// clang-format on
#include "quick/memory/Memcpy.cc"
#include "quick/memory/Memmove.cc"
#include "quick/memory/Strcpy.cc"

#include <gtest/gtest.h>
#include <print>
#include <string_view>
// clang-format off

class MemoryTest : public ::testing::Test
{ };

TEST_F(MemoryTest, MemmoveOverlappingRegions)
{
    char arr[]{"himom"};
    std::print("Before copy: arr=\"{}\"\n", arr);
    Memmove(arr + 2, arr, 2); // Copy "hi" to position 2
    std::print("After copy:  arr=\"{}\"\n", arr);
    EXPECT_EQ(std::string_view(arr), "hihim");
}

TEST_F(MemoryTest, StrcpyBasicCopy)
{
    const char arr[] = "12hi34";
    char *p = new char[sizeof(arr)]();

    std::print("Before copy: p=\"{}\"\n", p);
    Strcpy(p, arr);
    std::print("After copy:  p=\"{}\"\n", p);
    EXPECT_EQ(std::string_view(p), "12hi34");

    delete[] p;
}

TEST_F(MemoryTest, MemcpyLongString)
{
    const char arr[]{"reallyLongStringYouWouldntEvenBelieveItCuzzin"};
    // Use char array instead of std::byte to avoid strict aliasing issues
    char *p = ::new (std::nothrow) char[100]();
    std::println("Before:\tp=\t\"{}\"",
                 p); // char and unsigned char have special exemption from strict aliasing rules (C++ standard §6.7.2.1)
    std::println("Before:\tarr=\t\"{}\"", arr);
    std::println("Size: {}", sizeof(arr));

    Memcpy(static_cast<void *>(p), static_cast<const void *>(arr), sizeof(arr)); // Include null terminator
    std::print("After:\tp=\t\"{}\"\n", p);
    EXPECT_EQ(std::string_view(p), arr);
    
    ::delete[] p;
}