#pragma once

#include "order_book.hpp"
#include <iostream>

/**
 * @brief Utility class for printing the current state of the order book.
 */
class BookPrinter {
public:
    /**
     * @brief Prints the sell and buy sides of the order book to stdout.
     * 
     * @param book Reference to the current order book
     */
    static void print(const OrderBook& book) {
        std::cout << "----- ORDER BOOK -----\n";

        // Print sell side (ascending order)
        std::cout << "[SELL ORDERS]\n";
        for (const auto& [price, orders] : book.sell_orders()) {
            std::cout << "Price " << price << ": ";
            for (const auto& order : orders) {
                std::cout << order.quantity() << " ";
            }
            std::cout << "\n";
        }

        // Print buy side (descending order)
        std::cout << "\n[BUY ORDERS]\n";
        for (auto it = book.buy_orders().rbegin(); it != book.buy_orders().rend(); ++it) {
            const auto& price = it->first;
            const auto& orders = it->second;
            std::cout << "Price " << price << ": ";
            for (const auto& order : orders) {
                std::cout << order.quantity() << " ";
            }
            std::cout << "\n";
        }

        std::cout << "----------------------\n";
    }
};
