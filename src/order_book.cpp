#define NOMINMAX
#include "order_book.hpp"
#include <algorithm>

void OrderBook::add_order(const Order& order) {
    auto& book = (order.side() == OrderSide::BUY) ? buy_orders_ : sell_orders_;
    book[order.price()].push_back(order);
}

std::vector<Order> OrderBook::match_order(Order& incoming) {
    std::vector<Order> matched;
    int quantity_remaining = incoming.quantity();

    // Shared inner matching logic for one price level
    auto match_queue = [&](std::deque<Order>& queue) {
        while (!queue.empty() && quantity_remaining > 0) {
            Order& top = queue.front();
            int traded_quantity = std::min(quantity_remaining, top.quantity());

            if (traded_quantity == top.quantity()) {
                matched.push_back(std::move(top));
                queue.pop_front();
            } else {
                Order consumed = std::move(top);
                queue.pop_front();

                consumed.set_quantity(traded_quantity);
                matched.push_back(std::move(consumed));

                Order remainder(
                    top.id(), top.client_id(),
                    top.price(), top.quantity() - traded_quantity,
                    top.side()
                );
                queue.push_front(std::move(remainder));
            }

            quantity_remaining -= traded_quantity;
        }
    };

    if (incoming.side() == OrderSide::BUY) {
        auto it = sell_orders_.begin();
        while (it != sell_orders_.end() && quantity_remaining > 0) {
            if (it->first > incoming.price()) break;

            match_queue(it->second);

            if (it->second.empty()) {
                it = sell_orders_.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        auto it = buy_orders_.rbegin();
        while (it != buy_orders_.rend() && quantity_remaining > 0) {
            if (it->first < incoming.price()) break;

            match_queue(it->second);

            if (it->second.empty()) {
                it = std::make_reverse_iterator(
                    buy_orders_.erase(std::next(it).base())
                );
            } else {
                ++it;
            }
        }
    }

    incoming.set_quantity(quantity_remaining);
    return matched;
}
