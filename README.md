# Order Matching Engine – C++ Systems Simulation

This project is a simulation of a high-performance **Order Matching Engine**, implemented in **modern C++17**. It models a concurrent client-server architecture inspired by real-world financial trading systems. The goal is to simulate how market participants submit orders, and how a centralized matching engine processes these orders using standard industry practices such as **price-time priority**, **partial fills**, and **multithreaded execution**.

---
## Key Concepts Demonstrated

- **Multithreading and Synchronization**
  - `std::thread` for concurrent execution
  - `std::mutex`, `std::lock_guard`, and `std::condition_variable` for synchronization
  - `std::atomic<bool>` used for coordinated shutdown and run-state control

- **Socket Programming**
  - TCP server and client implemented using **WinSock2**
  - Per-client socket management using `accept()` and a dedicated thread per connection

- **Templates and Generic Programming**
  - `ThreadSafeQueue<T>` implemented as a reusable, templated blocking queue
  - Supports both lvalue and rvalue references with overloaded `push()` methods

- **Move Semantics**
  - Use of `std::move`, `T&&`, and move constructors for efficient resource transfer
  - Employed in queue operations and partial order fills to avoid deep copies

- **Smart Pointers**
  - `std::unique_ptr<T>` used to manage dynamically created orders (`parse_order`)
  - Ensures ownership semantics and prevents memory leaks

- **Modern C++ Language Features**
  - **Uniform initialization** with curly braces `{}` for consistent and safe construction
  - `std::optional<T>` for non-blocking queue retrieval
  - Lambda expressions for inline logic (e.g., filtering, callbacks)
  - reverse iterators for clean container traversal

- **Separation of Concerns**
  - Logical separation between order processing, networking, logging, and data modeling
  - Each component is testable and modular


## Overview

The system is structured around the following key components:

- **Client**
  - A simple CLI client that allows users to submit structured limit orders over TCP.
  - Each client runs in a separate terminal and maintains an open socket connection to the server.
  - Messages are sent in CSV format and parsed on the server side.
  - Each client asynchronously receives trade confirmations.

- **Order Matching Server**
  - Multi-threaded TCP server using **WinSock**.
  - Assigns a unique socket per client and maintains a mapping of client IDs to sockets.
  - Receives client orders and pushes them to the matching engine via a **thread-safe queue**.
  - Sends matched trade results back to both the buyer and seller.
  - Logs all trades to a central CSV file.

- **Matching Engine**
  - Processes orders in a separate thread.
  - Matches incoming orders against the opposite side of the order book using **price-time priority**.
  - Supports full and partial fills.
  - Adds any unmatched remainder to the appropriate side of the book.
  - Publishes matched trades to the server via another thread-safe queue.

- **Order Book**
  - Maintains buy and sell orders grouped by price.
  - Buy orders are stored in descending order of price; sell orders in ascending.
  - Uses `std::map<double, std::deque<Order>>` for both sides.

- **Thread-Safe Queue**
  - Custom templated blocking queue implemented with `std::mutex` and `std::condition_variable`.
  - Used for inter-thread communication between the server and matching engine.

- **Trade Logging**
  - All trades are saved to a structured `.csv` file for post-simulation review.

---

## Concurrency & Communication

- Each client is handled by a separate thread on the server.
- The matching engine runs as its own thread and continuously consumes orders from a blocking queue.
- Trade results are pushed to another queue and sent asynchronously to both clients involved in the trade.
- Shared resources such as socket maps and trade logs are protected using `std::mutex`.

---

## Key Components

- `order.hpp`: Defines the structure and behavior of an order.
- `order_book.hpp / .cpp`: Manages buy/sell books and matching logic.
- `matching_engine.hpp / .cpp`: Runs the matching loop in a background thread.
- `thread_safe_queue.hpp`: Generic queue for safe inter-thread communication.
- `order_server.hpp / .cpp`: Multi-threaded socket server managing client connections.
- `trade.hpp`: Represents a matched trade.
- `book_printer.hpp`: Utility to print the current state of the order book.
- `client.cpp`: Simple interactive CLI client.

---

## Notes

- The server uses **WinSock** and is designed for Windows environments.
- The client and server must be run in separate terminals.
- Messages are newline-terminated to allow line-by-line parsing.
- All trades are logged with client IDs, price, and quantity.

---

## Build & Execution

The system is written in standard C++17 and compiled using `g++` with `-lws2_32` and `-pthread`. Each component (engine and client) is built and run separately.

> This project is designed for demonstration purposes and highlights the use of multithreading, client-server networking, and low-level systems programming in modern C++.
