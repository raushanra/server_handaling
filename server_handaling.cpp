#include <iostream>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        exit(EXIT_FAILURE);
    }
    SOCKET server_fd, new_socket;
#else
    int server_fd, new_socket;
#endif
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    std::string html_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html><body><h1>Hello from C++ Web Server!</h1></body></html>";

    // Create socket
#ifdef _WIN32
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
#else
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#endif
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        // Accept connection
#ifdef _WIN32
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
#else
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
#endif
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Read request
        std::string request;
        while (true) {
            int valread = recv(new_socket, buffer, BUFFER_SIZE, 0);
            if (valread <= 0) break;
            request += std::string(buffer, valread);
            if (request.find("\r\n\r\n") != std::string::npos) break;  // End of headers
        }

        std::cout << "Request: " << request.substr(0, 100) << "..." << std::endl;

        // Simple parse for GET /
        std::string response;
        if (request.find("GET / ") != std::string::npos || request.find("GET /index.html ") != std::string::npos) {
            if (request.find("/index.html") != std::string::npos) {
                response = html_response;
            } else {
                response = 
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n\r\n"
                    "Hello World from C++ Socket Server!";
            }
        } else {
            response = 
                "HTTP/1.1 404 Not Found\r\n\r\n"
                "Page not found.";
        }

        // Send response
        send(new_socket, response.c_str(), response.length(), 0);

        // Close connection
#ifdef _WIN32
        closesocket(new_socket);
#else
        close(new_socket);
#endif
    }

#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
    return 0;
}
