#include "matching_engine.hpp"

void MatchingEngine::run() {
    while (running_) {
        Order order;
        in_queue_.wait_and_pop(order);  // Blocks until an order arrives

        if (order.id() == "__SHUTDOWN__") {
            break;  // Special shutdown signal
        }

        // Match the incoming order against the order book
        auto matched_orders = book_.match_order(order);

        // For each match, publish a Trade
        for (Order& top : matched_orders) {
            Trade trade(
                (order.side() == OrderSide::BUY) ? order.client_id() : top.client_id(),
                (order.side() == OrderSide::SELL) ? order.client_id() : top.client_id(),
                top.price(),
                top.quantity()
            );
            trade_queue_.push(std::move(trade));
        }

        // Add remaining unmatched portion to the book
        if (order.quantity() > 0) {
            book_.add_order(order);
        }
    }
}

void MatchingEngine::stop() {
    running_ = false;
}

OrderBook& MatchingEngine::book() {
    return book_;
}
