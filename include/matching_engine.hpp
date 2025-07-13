#pragma once

#include "order.hpp"
#include "trade.hpp"
#include "order_book.hpp"
#include "thread_safe_queue.hpp"
#include <atomic>

/**
 * @brief Core matching engine.
 *        Pulls orders from an input queue, matches them,
 *        pushes resulting trades to an output queue,
 *        and manages the internal order book.
 */
class MatchingEngine {
public:
    using OrderQueue = ThreadSafeQueue<Order>;
    using TradeQueue = ThreadSafeQueue<Trade>;

    MatchingEngine(OrderQueue& in, TradeQueue& out)
        : in_queue_(in), trade_queue_(out) {}

    /// Starts the matching loop (blocking call)
    void run();

    /// Signals the engine to stop
    void stop();

    /// Access the internal order book (for diagnostics/logging)
    OrderBook& book();

private:
    std::atomic<bool> running_{true};
    OrderQueue& in_queue_;
    TradeQueue& trade_queue_;
    OrderBook book_;
};
