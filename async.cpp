#include <iostream>
#include <pthread.h>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <condition_variable>
#include <mutex>

#include <functional>


class ThreadPool {
public:
	//1.A. THREAD POOL WAS CREATED EARLIER WITH ITS WORKER FUNCTION
    ThreadPool(int numThreads) : numThreads_(numThreads), stop_(false) {
        threads_.resize(numThreads_);
        for (int i = 0; i < numThreads_; ++i) {
            pthread_create(&threads_[i], nullptr, workerThread, this);
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (pthread_t& thread : threads_) {
            pthread_join(thread, nullptr);
        }
    }

    void addTask(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            tasks_.push(task);
        }
        condition_.notify_one();
    }

private:
	//3. WORKER FUNCTION PERPETUALLY CHECKS FOR TASKS AND EXECUTES THEM
    static void* workerThread(void* arg) {
        ThreadPool* pool = static_cast<ThreadPool*>(arg);
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(pool->queueMutex_);
                pool->condition_.wait(lock, [pool] { 
					return pool->stop_ || !pool->tasks_.empty(); });
                
				 if (pool->stop_ && pool->tasks_.empty()) {
                    return nullptr;
                }
                task = pool->tasks_.front();
                pool->tasks_.pop();
            }
            task();
        }
    }

    int numThreads_;
    std::vector<pthread_t> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_;
};

int main() {
	//1. CREATE THREADS WITH WORKER FUNCTION WHICH CALLS TASKS
    ThreadPool pool(4);
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server listening on port 8080" << std::endl;

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }
		//2. ADD TASKS TO THE THREAD POOL OBJECT JUST CREATED
        pool.addTask([clientSocket]() {
            char buffer[1024] = {0};
            recv(clientSocket, buffer, 1024, 0);
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello, World!</h1></body></html>";
            send(clientSocket, response.c_str(), response.size(), 0);
            close(clientSocket);
            });
    }
    close(serverSocket);
    return 0;
}
