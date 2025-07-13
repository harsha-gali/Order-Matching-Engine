#pragma once

#include <string>
#include <sstream>

/**
 * @brief Represents a completed trade between a buyer and a seller.
 */
struct Trade {
    std::string buy_client_id;   ///< ID of the buying client
    std::string sell_client_id;  ///< ID of the selling client
    double price;                ///< Execution price
    int quantity;                ///< Quantity traded


    Trade(const std::string& buy, const std::string& sell, double pr, int qty)
        : buy_client_id(buy), sell_client_id(sell), price(pr), quantity(qty) {}

    /**
     * @brief Converts the trade to a human-readable string.
     * 
     * @return A formatted string describing the trade
     */
    std::string to_string() const {
        std::ostringstream oss;
        oss << "TRADE: " << quantity << " @ " << price
            << " [BUYER: " << buy_client_id
            << ", SELLER: " << sell_client_id << "]";
        return oss.str();
    }
};
