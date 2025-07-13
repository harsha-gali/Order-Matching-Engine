#include <iostream>
#include <string>
#include <thread>
#include <atomic>

#include "../include/platform.hpp"


std::atomic<bool> running{true};

/**
 * @brief Receives and prints messages from the server.
 * 
 * @param sock Connected socket to the server
 */
void receive_messages(SOCKET sock) {
    char buffer[1024];

    while (running) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::cout << "\n[Server]: " << buffer << "\n> ";
            std::cout.flush();
        } else if (bytes == 0) {
            std::cout << "\n[Info] Server closed the connection.\n";
            running = false;
            break;
        } else {
            std::cerr << "\n[Error] Failed to receive from server.\n";
            running = false;
            break;
        }
    }
}

int main() {
    SOCKET sock = INVALID_SOCKET;
    const char* server_ip = "127.0.0.1";
    const int port = 54000;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Error] WSAStartup failed.\n";
        return 1;
    }
#endif

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "[Error] Socket creation failed.\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Configure server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &serverAddr.sin_addr);

    // Connect to server
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[Error] Failed to connect to server.\n";
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "Connected to " << server_ip << ":" << port << "\n";
    std::cout << "Enter orders in format: CLIENT_ID,PRICE,QUANTITY,SIDE\n";
    std::cout << "Example: B1,101.5,10,BUY\n";
    std::cout << "Type 'exit' to quit.\n\n";

    // Start receive thread
    std::thread recv_thread(receive_messages, sock);

    std::string line;
    while (running) {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line == "exit") {
            running = false;
            break;
        }

        line += "\n";
        int sent = send(sock, line.c_str(), static_cast<int>(line.length()), 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "[Error] Failed to send message.\n";
            break;
        }
    }

    shutdown(sock, SD_BOTH);
    closesocket(sock);

    if (recv_thread.joinable()) recv_thread.join();

#ifdef _WIN32
    WSACleanup();
#endif

    std::cout << "Disconnected from server.\n";
    return 0;
}
