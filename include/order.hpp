#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <cstdint>


// --- Enums for order direction and type ---
enum class OrderSide { BUY, SELL };
enum class OrderType { LIMIT };

// --- Lightweight utility functions ---
inline std::string to_string(OrderSide side) {
    return (side == OrderSide::BUY) ? "BUY" : "SELL";
}

inline OrderSide parse_order_side(const std::string& str) {
    if (str == "BUY") return OrderSide::BUY;
    if (str == "SELL") return OrderSide::SELL;
    throw std::invalid_argument("Invalid OrderSide: " + str);
}

inline std::string generate_order_id() {
    static uint64_t counter = 0;
    return "ORD" + std::to_string(++counter);
}

inline std::chrono::system_clock::time_point current_timestamp() {
    return std::chrono::system_clock::now();
}

// --- Core Order class ---
class Order {
public:
    Order();
    Order(std::string client_id, double price, int quantity, OrderSide side, OrderType type = OrderType::LIMIT);
    Order(std::string id, std::string client_id, double price, int quantity, OrderSide side, OrderType type = OrderType::LIMIT);
    virtual ~Order() ;

    const std::string& id() const { return id_; }
    const std::string& client_id() const { return client_id_; }
    OrderSide side() const { return side_; }
    int quantity() const { return quantity_; }
    double price() const { return price_; }
    OrderType type() const { return type_; }
    std::chrono::system_clock::time_point timestamp() const { return timestamp_; }

    void set_quantity(int q) { quantity_ = q; }

    virtual std::string to_string() const;

private:
    std::string id_;
    std::string client_id_;
    OrderSide side_;
    int quantity_;
    double price_;
    OrderType type_;
    std::chrono::system_clock::time_point timestamp_;
};

// --- Parses a command-line string into an Order object ---
// Format: "BUY 5 100.0" or "SELL 10 101.5"
std::unique_ptr<Order> parse_order(const std::string& client_id, const std::string& input_line);
