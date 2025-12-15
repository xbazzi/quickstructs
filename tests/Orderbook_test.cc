
// clang-format on
#include "quick/structs/Orderbook.hh"

#include <algorithm>
#include <gtest/gtest.h>
#include <iostream>
#include <print>
#include <ranges>
// clang-format off

class OrderbookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup code if needed
    }
};

TEST_F(OrderbookTest, AddOrdersAndGenerateTrades)
{
    Orders orders;
    orders.reserve(2);
    Id id = 1;
    Price price = 1;
    Quantity qty = 4;
    bool isbuy = true;

    constexpr Quantity magic = 10;
    std::size_t n_orders = magic * 2;

    std::generate_n(std::back_inserter(orders), n_orders, [&]() mutable {
        bool currentisbuy = isbuy;
        isbuy = !isbuy;
        return Order{++id, price++, currentisbuy, qty++};
    });

    // OR
    // auto gen = std::views::iota(0U) | std::views::take(n_orders) |
    //            std::views::transform(
    //                [=](auto i) mutable
    //                {
    //                    bool isbuy = (i % 2) == 0;
    //                    //    isbuy             = !isbuy;
    //                    return Order{ id + i, price + i, isbuy, qty + i };
    //                });

    // std::ranges::copy(gen, std::back_inserter(orders));
    std::for_each(orders.begin(), orders.end(), [](const Order &o) { std::cout << o << std::endl; });
    
    Orderbook orderbook;
    Trades trades;

    for (auto it{std::begin(orders)}; it != std::end(orders); ++it)
    {
        Trades new_trades{orderbook.AddOrder(*it)};
        std::ranges::copy(new_trades, std::back_inserter(trades));
    }

    // zip all trades vectors and print them out
    //
    // std::vector<std::reference_wrapper<const Trade>> all_trades;
    std::for_each(trades.begin(), trades.end(), [&](const Trade &t) { 
        std::cout << "Hi: " << t.Level << std::endl; 
    });

    EXPECT_FALSE(trades.empty());
}