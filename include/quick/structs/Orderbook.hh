#include <algorithm>
#include <cctype>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

using Id = size_t;
using Price = long;
using Quantity = int;

class Order
{
  public:
    enum class Side
    {
        BUY,
        SELL
    };

  private:
    Id id_;
    Price level_;
    bool is_buy_;
    Quantity qty_;
    Side side_;

  public:
    Order(Id orderId, Price level, bool isBuy, Quantity quantity)
        : id_{orderId}, level_{level}, is_buy_{isBuy}, qty_{quantity}, side_{isBuy ? Side::BUY : Side::SELL}
    {
    }

    friend std::ostream &operator<<(std::ostream &os, const Order &o)
    {
        os << "id: " << o.get_id() << ", "
           << "level: " << o.get_level() << ", "
           << "isbuy: " << o.is_buy() << ", "
           << "qty: " << o.get_qty();
        return os;
    }

    bool is_buy() const noexcept
    {
        return is_buy_;
    }

    Id OrderId() const
    {
        return id_;
    }

    Side get_side() const noexcept
    {
        return side_;
    }

    Quantity get_qty() const
    {
        return qty_;
    }

    Id get_id() const
    {
        return id_;
    }

    Price get_level() const
    {
        return level_;
    }

    void set_qty(Quantity new_qty)
    {
        qty_ = static_cast<Quantity>(new_qty);
    }
};

using Orders = std::vector<Order>;

// DO NOT MODIFY.
struct Trade
{
    Id OrderIdA;
    Id OrderIdB; // Aggressor's OrderId
    Id AggressorOrderId;
    bool AggressorIsBuy;
    Price Level;
    Quantity Size;
};

using Trades = std::vector<Trade>;

class Orderbook
{
    Orders bids_{};
    Orders asks_{};

  public:
    static constexpr inline std::uint64_t reserved_size_ = 20UL;

    Orderbook()
    {
        bids_.reserve(reserved_size_);
        asks_.reserve(reserved_size_);
    }

    bool can_match(const Order &buy, const Order &sell)
    {
        return (buy.get_level() >= sell.get_level());
    }

    bool is_dupe(const Order &order)
    {
        auto is_match = [&order](Order &o) { return (o.get_id() == order.get_id()); };

        return std::ranges::any_of(bids_, is_match) || std::ranges::any_of(asks_, is_match);
    }

    void insert_order(const Order &order)
    {
        if (is_dupe(order))
            return;
        if (order.is_buy())
        {
            // lower bound because we want to respect orders at the same
            // price level that came first, and those should be closer to back
            // so they get picked up first
            auto pos = std::ranges::lower_bound(bids_, order.get_level(), std::less<Price>(), &Order::get_level);
            bids_.insert(pos, order);
            return;
        }
        auto pos = std::ranges::lower_bound(asks_, order.get_level(), std::greater<Price>(), &Order::get_level);
        asks_.insert(pos, order);
    }

    [[nodiscard]]
    Trades AddOrder(const Order &incoming)
    {
        Trades trades;
        if (is_dupe(incoming))
            return trades;

        auto &opposite_side = incoming.is_buy() ? asks_ : bids_;
        // auto& same_side     = incoming.is_buy() ? bids_ : asks_;

        Quantity remaining = incoming.get_qty();

        while (!opposite_side.empty() and remaining > 0)
        {
            Order &best = opposite_side.back();
            bool match = incoming.is_buy() ? can_match(incoming, best) : can_match(best, incoming);
            if (!match)
                break;

            // figure out how much qty from incoming we can trade
            Quantity trade_size = std::min(best.get_qty(), remaining);

            // Price is ALWAYS from the ask (sell) side
            // If incoming is buy, best is sell (ask) - use best.get_level()
            // If incoming is sell, incoming is ask - use incoming.get_level()
            Price trade_price = incoming.is_buy() ? best.get_level() : incoming.get_level();

            // Determine aggressor: incoming is always the aggressor in this
            // matching model
            Id aggressor_id = incoming.get_id();
            bool aggressor_is_buy = incoming.is_buy();

            // Trade format: bid order first, then ask order
            Id bid_order_id = incoming.is_buy() ? incoming.get_id() : best.get_id();
            Id ask_order_id = incoming.is_buy() ? best.get_id() : incoming.get_id();

            // send trade since there is a match
            trades.emplace_back(
                Trade{bid_order_id, ask_order_id, aggressor_id, aggressor_is_buy, trade_price, trade_size});

            // update quantities
            best.set_qty(best.get_qty() - trade_size);
            remaining -= trade_size;

            // get rid of fully-filled opposite-side order (remove last element)
            if (best.get_qty() == 0)
                opposite_side.pop_back();
        }

        if (remaining > 0)
        {
            Order corrected_incoming = incoming;
            corrected_incoming.set_qty(remaining);
            insert_order(corrected_incoming);
        }
        return trades;
    }

    void CancelOrder(Id order_id)
    {
        auto id_match = [=](const Order &o) { return o.get_id() == order_id; };
        auto bid_it = std::ranges::find_if(bids_, id_match);
        if (bid_it != std::end(bids_))
        {
            bids_.erase(bid_it);
            return;
        }
        auto asks_it = std::ranges::find_if(asks_, id_match);
        if (asks_it != std::end(asks_))
        {
            asks_.erase(asks_it);
            return;
        }
    }
};
