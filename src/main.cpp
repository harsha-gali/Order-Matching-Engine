#include "../include/matching_engine.hpp"
#include "../include/order_server.hpp"
#include "../include/book_printer.hpp"
#include <iostream>
#include <thread>

int main() {
    // Thread-safe queues
    ThreadSafeQueue<Order> order_input_queue;
    ThreadSafeQueue<Trade> trade_output_queue;

    // 1. Start the matching engine
    MatchingEngine engine(order_input_queue, trade_output_queue);
    std::thread engine_thread(&MatchingEngine::run, &engine);

    // 2. Start the TCP order server
    OrderServer server(order_input_queue, 54000);
    server.set_trade_queue(trade_output_queue);
    server.start();

    std::cout << "Order Matching Engine and TCP server started.\n";
    std::cout << "Clients can now connect and submit orders.\n";
    std::cout << "Press Enter to stop the system...\n";
    std::cin.get();

    // 3. Shutdown sequence
    std::cout << "Shutting down...\n";

    

    // Send shutdown order to unblock engine  
    order_input_queue.push(Order("__SHUTDOWN__", 0.0, 0, OrderSide::BUY));
    std::cout << "Shutting down engine loop\n";
    engine.stop();  // Stop matching engine loop
    std::cout << "Shutting down client handling threads\n";
    server.stop();  // Stop client handling threads
    std::cout << "Waiting for engine_thread to join\n";
    engine_thread.join();

    // 4. Optionally print order book
    BookPrinter::print(engine.book());

    // 5. Save trade log to CSV
    server.write_trade_log_to_file("trade_log.csv");

    std::cout << "All done. Goodbye.\n   ";
    return 0;
}
