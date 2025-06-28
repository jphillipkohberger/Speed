/*
A multithreaded asynchronous web server in C++ can be implemented using techniques that allow the server to handle multiple client requests concurrently without blocking. This approach typically involves using threads to manage connections and asynchronous I/O operations to prevent blocking on network operations. Here's a breakdown of how it can be achieved:
1. Thread Management
Thread Pool:
A common approach is to use a thread pool, which is a collection of pre-initialized threads that are ready to execute tasks. When a new client connection is established, a thread from the pool is assigned to handle the connection. Once the task is completed, the thread returns to the pool, ready for the next connection.
Thread Creation per Connection:
Another approach, although less efficient for high-load scenarios, is to create a new thread for each incoming connection. This can lead to excessive thread creation and destruction overhead.
2. Asynchronous I/O
Non-blocking Sockets:
The server should use non-blocking sockets to handle network I/O. This allows the server to initiate a read or write operation and return immediately without waiting for the operation to complete.
Event Notification:
Mechanisms like epoll (Linux), kqueue (BSD/macOS), or I/O completion ports (Windows) can be used to monitor multiple sockets for events (e.g., data available for reading, socket ready for writing). When an event occurs, the server can then handle the corresponding socket without blocking.
3. Request Handling
Asynchronous Processing: Once a request is received, it should be processed asynchronously to avoid blocking the thread handling the connection. This can be achieved by using techniques like:
Callbacks: Registering callback functions to be executed when an asynchronous operation completes.
Futures and Promises: Using futures and promises to represent the result of an asynchronous operation.
Async/Await: Using the async and await keywords (available in C++20 and later) to write asynchronous code that resembles synchronous code.
*/ 

#include <boost/asio.hpp>

#include <iostream>

#include <thread>

#include <vector>


class ConnectionHandler {
  public: ConnectionHandler(boost::asio::ip::tcp::socket socket): socket_(std::move(socket)) {}

  void start() {
    read_request();
  }

  private: void read_request() {
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n",
      [this, self](const boost::system::error_code & error, size_t bytes_transferred) {
        if (!error) {
          process_request();
        }
      });
  }

  void process_request() {
    // Process the request data from buffer_
    std::string request_data {
      boost::asio::buffers_begin(buffer_.data()),
        boost::asio::buffers_begin(buffer_.data()) + buffer_.size()
    };
    std::cout << "Received request: " << request_data << std::endl;

    // Send a response
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
    boost::asio::async_write(socket_, boost::asio::buffer(response),
      [this](const boost::system::error_code & error, size_t bytes_transferred) {
        if (!error) {
          // Connection is complete, close the socket
          socket_.close();
        }
      });
  }

  private: boost::asio::ip::tcp::socket socket_;
  boost::asio::streambuf buffer_;
};

class Server {
  public: Server(boost::asio::io_context & io_context, short port): acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
  thread_pool_size_(4) {
    start_accept();
  }

  void run() {
    std::vector < std::thread > threads;
    for (size_t i = 0; i < thread_pool_size_; ++i) {
      threads.emplace_back([this] {
        io_context_.run();
      });
    }

    for (auto & thread: threads) {
      thread.join();
    }
  }

  private: void start_accept() {
    acceptor_.async_accept([this](const boost::system::error_code & error,
      boost::asio::ip::tcp::socket socket) {
      if (!error) {
        std::make_shared < ConnectionHandler > (std::move(socket)) -> start();
      }
      start_accept(); // Accept the next connection
    });
  }

  private: boost::asio::io_context io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
  size_t thread_pool_size_;
};

int main() {
  boost::asio::io_context io_context;
  Server server(io_context, 8080);
  server.run();

  return 0;
}