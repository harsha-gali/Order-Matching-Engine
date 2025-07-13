#pragma once

#include "order.hpp"
#include "trade.hpp"
#include "thread_safe_queue.hpp"

#include "platform.hpp"


#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>

/**
 * @brief TCP-based order server that accepts clients,
 *        receives incoming orders, and sends trade confirmations.
 */
class OrderServer {
public:
    /**
     * @param input_queue Thread-safe queue for submitting orders to the matching engine
     * @param port Listening port for incoming client connections (default: 54000)
     */
    OrderServer(ThreadSafeQueue<Order>& input_queue, int port = 54000);
    ~OrderServer();

    /// Starts the server: accepts clients and spawns handler threads
    void start();

    /// Signals the server to stop and joins all threads
    void stop();

    /// Sets the trade queue used for sending trade responses
    void set_trade_queue(ThreadSafeQueue<Trade>& trade_queue);

    /// Writes all completed trades to a CSV file
    void write_trade_log_to_file(const std::string& filename) const;

private:
    /// Accepts new clients and dispatches handlers
    void accept_clients();

    /// Handles individual client session (receiving orders)
    void handle_client(SOCKET client_socket);

    /// Sends matched trade confirmations back to clients
    void send_trade_responses();

private:
    int port_;
    SOCKET listen_socket_ = INVALID_SOCKET;
    std::atomic<bool> running_{true};

    // Map client ID → socket
    std::unordered_map<std::string, SOCKET> client_sockets_;
    mutable std::mutex socket_mutex_;

    ThreadSafeQueue<Order>& input_queue_;
    ThreadSafeQueue<Trade>* trade_queue_ = nullptr;

    std::thread accept_thread_;
    std::thread response_thread_;
    std::vector<std::thread> client_threads_;

    std::vector<Trade> trade_log_;
    mutable std::mutex log_mutex_;
};
