#include "order_server.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>

// Constants
constexpr int kBufferSize = 1024;
constexpr int kSleepMs = 10;

#include "platform.hpp"


OrderServer::OrderServer(ThreadSafeQueue<Order>& input_queue, int port)
    : input_queue_(input_queue), port_(port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
    }
#endif
}

OrderServer::~OrderServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

void OrderServer::set_trade_queue(ThreadSafeQueue<Trade>& trade_queue) {
    trade_queue_ = &trade_queue;
}

void OrderServer::start() {
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket.\n";
        return;
    }

    int opt = 1;
    setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));

    if (bind(listen_socket_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        return;
    }

    if (listen(listen_socket_, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        return;
    }

    accept_thread_ = std::thread(&OrderServer::accept_clients, this);
    response_thread_ = std::thread(&OrderServer::send_trade_responses, this);
}

void OrderServer::stop() {
    running_ = false;
    
    shutdown(listen_socket_, SHUT_RDWR);
    closesocket(listen_socket_);

    for (auto& t : client_threads_) {
        if (t.joinable()) t.join();
    }

    if (accept_thread_.joinable()) accept_thread_.join();
    if (response_thread_.joinable()) response_thread_.join();

}

void OrderServer::accept_clients() {
    while (running_) {
        SOCKET client_socket = accept(listen_socket_, nullptr, nullptr);
        if (client_socket != INVALID_SOCKET) {
            client_threads_.emplace_back(&OrderServer::handle_client, this, client_socket);
        }
    }

}

void OrderServer::handle_client(SOCKET client_socket) {
    char buffer[kBufferSize];
    int bytesReceived;

    while ((bytesReceived = recv(client_socket, buffer, kBufferSize - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        std::istringstream stream(buffer);
        std::string line;

        while (std::getline(stream, line)) {
            std::istringstream ss(line);
            std::string client_id, price_str, qty_str, side_str;

            if (std::getline(ss, client_id, ',') &&
                std::getline(ss, price_str, ',') &&
                std::getline(ss, qty_str, ',') &&
                std::getline(ss, side_str)) {

                try {
                    double price = std::stod(price_str);
                    int qty = std::stoi(qty_str);
                    OrderSide side = (side_str == "BUY") ? OrderSide::BUY : OrderSide::SELL;

                    Order order(client_id, price, qty, side);

                    {
                        std::lock_guard<std::mutex> lock(socket_mutex_);
                        client_sockets_[client_id] = client_socket;
                    }

                    input_queue_.push(std::move(order));
                } catch (const std::exception& e) {
                    std::cerr << "Invalid order format: " << line << "\n";
                }
            }
        }
    }

    closesocket(client_socket);
}

void OrderServer::send_trade_responses() {
    while (running_) {
        if (!trade_queue_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(kSleepMs));
            continue;
        }

        auto trade = trade_queue_->try_pop();
        if (trade) {
            std::string msg = trade->to_string() + "\n";

            {
                std::lock_guard<std::mutex> lock(socket_mutex_);
                auto send_to = [&](const std::string& client_id) {
                    auto it = client_sockets_.find(client_id);
                    if (it != client_sockets_.end()) {
                        SOCKET sock = it->second;
                        send(sock, msg.c_str(), static_cast<int>(msg.length()), 0);
                    }
                };

                send_to(trade->buy_client_id);
                send_to(trade->sell_client_id);
            }

            {
                std::lock_guard<std::mutex> lock(log_mutex_);
                trade_log_.push_back(*trade);
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(kSleepMs));
        }
    }

}

void OrderServer::write_trade_log_to_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open trade log file: " << filename << "\n";
        return;
    }

    file << "BuyClientID,SellClientID,Price,Quantity\n";

    std::lock_guard<std::mutex> lock(log_mutex_);
    for (const auto& trade : trade_log_) {
        file << trade.buy_client_id << ","
             << trade.sell_client_id << ","
             << trade.price << ","
             << trade.quantity << "\n";
    }

    std::cout << "Trade log saved to " << filename << "\n";
}
