/*
C++ enables the creation of multithreaded asynchronous socket programs, allowing for concurrent handling of multiple client connections. This approach enhances the responsiveness and scalability of network applications.
Multithreading and Asynchronous Operations
Multithreading involves creating multiple threads within a process, enabling concurrent execution of different parts of the program. Asynchronous operations, on the other hand, allow a program to initiate a task and continue processing other tasks without waiting for the initial task to complete. When combined with sockets, these techniques facilitate non-blocking communication, where the program can handle multiple socket connections simultaneously.
Implementation Approaches
Thread per Connection:
For each incoming client connection, a new thread is created to handle communication.
This model is straightforward to implement but can become resource-intensive with a large number of connections.
Synchronization mechanisms, such as mutexes or atomic variables, are necessary to manage shared resources and prevent race conditions.
Thread Pool:
A pool of threads is maintained to handle incoming connections.
When a new connection arrives, a thread from the pool is assigned to manage it.
This approach limits the number of threads created, improving resource management compared to the thread-per-connection model.
Asynchronous I/O with select or poll:
The select or poll system calls can be used to monitor multiple sockets for activity.
A single thread can manage multiple connections by checking which sockets are ready for reading or writing.
This approach is efficient for handling a large number of connections with minimal overhead.
Asynchronous I/O with Boost.Asio:
Boost.Asio is a powerful C++ library that provides a framework for asynchronous programming.
It allows for efficient handling of multiple socket connections using an event-driven approach.
Asio simplifies the implementation of asynchronous operations and provides features like timers and signals.

This example demonstrates a basic server that creates a new thread for each incoming connection. The handle_connection function receives data from the client and sends it back. The main loop continuously accepts new connections and spawns threads to manage them

*/

#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void handle_connection(int client_socket) {
    char buffer[1024];
    ssize_t bytes_received;
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        send(client_socket, buffer, bytes_received, 0);
    }
    close(client_socket);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        std::thread client_thread(handle_connection, client_socket);
        client_thread.detach();
    }
    return 0;
}
