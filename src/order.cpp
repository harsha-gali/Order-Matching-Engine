#include "order.hpp"
#include <sstream>
#include <iomanip>

Order::Order()
    : id_(""), price_(0.0), quantity_(0), side_(OrderSide::BUY) {}
    
Order::~Order() = default;

Order::Order(std::string client_id, double price, int quantity, OrderSide side, OrderType type)
    : id_(generate_order_id()),
      client_id_(std::move(client_id)),
      side_(side),
      quantity_(quantity),
      price_(price),
      type_(type),
      timestamp_(current_timestamp()) {}

Order::Order(std::string id, std::string client_id, double price, int quantity, OrderSide side, OrderType type)
    : id_(std::move(id)),
      client_id_(std::move(client_id)),
      price_(price),
      quantity_(quantity),
      side_(side),
      type_(type),
      timestamp_(current_timestamp()) {}

std::string Order::to_string() const {
    std::ostringstream oss;
    oss << "[" << id_ << "] "
        << ::to_string(side_) << " "
        << quantity_ << " @ " << std::fixed << std::setprecision(2) << price_
        << " (Client: " << client_id_ << ")";
    return oss.str();
}

std::unique_ptr<Order> parse_order(const std::string& client_id, const std::string& input_line) {
    std::istringstream iss(input_line);
    std::string side_str;
    int quantity;
    double price;

    iss >> side_str >> quantity >> price;

    if (!iss || quantity <= 0 || price <= 0.0) {
        throw std::invalid_argument("Invalid order input: " + input_line);
    }

    OrderSide side = parse_order_side(side_str);
    return std::make_unique<Order>(client_id, price, quantity, side, OrderType::LIMIT);
}
