/*
A multithreaded asynchronous web server in C++ handles multiple client requests concurrently, maximizing resource utilization and improving responsiveness. It uses threads to manage multiple connections and asynchronous I/O operations to prevent blocking.
Here's a basic outline of how such a server can be implemented:
Socket Setup: Create a listening socket to accept incoming client connections.
Thread Pool: Initialize a pool of worker threads.
Accept Connections: Accept incoming client connections in the main thread.
Dispatch to Workers: For each accepted connection, dispatch the handling of the request to a worker thread from the pool.
Asynchronous I/O: Within the worker thread, use asynchronous I/O operations (e.g., async_read, async_write from Boost.Asio or similar) to handle the request without blocking the thread.
Request Processing: Process the client's request and generate a response.
Send Response: Send the response back to the client using asynchronous I/O.
Thread Management: Properly manage the lifecycle of threads and handle potential errors or exceptions
*/
#include <iostream>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <thread>
#include <vector>
#include <memory>

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(asio::ip::tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        read();
    }

private:
    void read() {
        auto self = shared_from_this();
        asio::async_read(socket_, asio::dynamic_buffer(buffer_),
            [self](const asio::error_code& error, std::size_t bytes_transferred) {
                if (!error) {
                    self->process_request();
                }
            });
    }

    void process_request() {
        // Process the request in buffer_ and prepare the response
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
        write(response);
    }

    void write(const std::string& response) {
        auto self = shared_from_this();
        asio::async_write(socket_, asio::buffer(response),
            [self](const asio::error_code& error, std::size_t bytes_transferred) {
                // Handle write completion or error
                self->socket_.close();
            });
    }

    asio::ip::tcp::socket socket_;
    std::string buffer_;
};

class Server {
public:
    Server(asio::io_context& io_context, short port, int thread_count)
        : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          thread_count_(thread_count) {}

    void start() {
        for (int i = 0; i < thread_count_; ++i) {
            threads_.emplace_back([this] { io_context_.run(); });
        }
        accept_connections();
    }

    void stop() {
        io_context_.stop();
        for (auto& thread : threads_) {
            thread.join();
        }
    }

private:
    void accept_connections() {
        acceptor_.async_accept(
            [this](const asio::error_code& error, asio::ip::tcp::socket socket) {
                if (!error) {
                    std::make_shared<Connection>(std::move(socket))->start();
                }
                accept_connections(); // Continue accepting new connections
            });
    }

    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> threads_;
    int thread_count_;
};

int main() {
    asio::io_context io_context;
    Server server(io_context, 8080, 4); // Start server on port 8080 with 4 threads
    server.start();
    std::cin.get(); // Keep the server running until Enter is pressed
    server.stop();
    return 0;
}
