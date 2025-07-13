#pragma once

#include <map>
#include <deque>
#include <vector>
#include <memory>
#include "order.hpp"

/**
 * @brief Manages a limit order book.
 * 
 * Stores buy and sell orders in price levels, and matches incoming orders
 * against the opposite side of the book using price-time priority.
 */
class OrderBook {
public:
    /**
     * @brief Add an unmatched order to the appropriate side of the book.
     */
    void add_order(const Order& order);

    /**
     * @brief Attempt to match an incoming order with the opposite side.
     * 
     * @param incoming The order to match
     * @return A list of matched (consumed) orders
     */
    std::vector<Order> match_order(Order& incoming);

    /**
     * @return Read-only access to current buy-side levels
     */
    const auto& buy_orders() const { return buy_orders_; }

    /**
     * @return Read-only access to current sell-side levels
     */
    const auto& sell_orders() const { return sell_orders_; }

private:
    // Buy side (stored lowest→highest, but accessed in reverse to get highest price first)
    std::map<double, std::deque<Order>> buy_orders_;

    // Sell side: lowest price first
    std::map<double, std::deque<Order>> sell_orders_;
};
