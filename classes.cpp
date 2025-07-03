#ifndef CLASSES
#define CLASSES

#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cerrno>
#include <system_error>
#include <tuple>
#include <pthread.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <queue>
#include <netinet/in.h>
#include <condition_variable>
#include <mutex>
#include <map>
#include <functional>
#include <cerrno>
#include <system_error>
#include <cstdlib>

#define PORT_ONE 8080
#define BUFFER_SIZE 1024
#define WEB_ROOT "../web" //relative to this file
#define THREAD_POOL_SIZE 5
#define TASK_COUNT 10

pthread_mutex_t _mutex;

class Task {

public:

	int id;
	
	void task_work() {
	
	};

};

class ThreadPool {

public:

	pthread_t threads[THREAD_POOL_SIZE];
	Task *tasks;
	int task_count;
	int task_index;
	bool stop_ = false;
	int num_threads_;
	std::vector<pthread_t> threads_;
	std::queue<std::function<void()>> tasks_;
	std::vector<std::string> tasks_counter;
	std::mutex queueMutex_;
   	std::condition_variable condition_;

	pthread_mutex_t queue_mutex;
	pthread_cond_t queue_not_empty;
 	pthread_cond_t queue_not_full;
	pthread_mutex_t mutex;
 	pthread_cond_t cond;

	ThreadPool (int num_threads) {
	
		this->num_threads_ = num_threads;
		
		this->threads_.resize(this->num_threads_);
		
		pthread_mutex_init(&this->queue_mutex, NULL);
    	pthread_cond_init(&this->queue_not_empty, NULL);
    	pthread_cond_init(&this->queue_not_full, NULL);

		for (int i = 0; i < this->num_threads_; ++i) {
			
       		pthread_create(
				&this->threads_[i], nullptr, thread_pool_worker, this);
      	}

	}
	
	static void* thread_pool_worker(void* arg) {
	
		ThreadPool* pool = static_cast<ThreadPool*>(arg);
		
		while (true) {
		
			pthread_mutex_lock(&pool->queue_mutex);
			
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(pool->queueMutex_);
               pool->condition_.wait(
					lock, [pool] { 
					return pool->stop_ || !pool->tasks_.empty(); 
				});
				if (pool->stop_ && pool->tasks_.empty()) {
                 	return nullptr;
            }

			task = pool->tasks_.front();
           	pool->tasks_.pop();
			}
			
			// Signal that queue is not full
        	pthread_cond_signal(&pool->queue_not_full);
        	pthread_mutex_unlock(&pool->queue_mutex);
				
           task();
        }
		return nullptr;	
    }
	
	void add_thread(void*(*callback)(void*), void *arg) {
	
		pthread_t thread;
		
		pthread_create(&thread, nullptr, callback, arg);
		
		pthread_join(thread,nullptr);
	
	}
	
	void add_task(std::function<void()> task) {
		pthread_mutex_lock(&this->queue_mutex);
       	{
            std::unique_lock<std::mutex> lock(queueMutex_);
            tasks_.push(task);
       }
       condition_.notify_one();

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

		pthread_mutex_destroy(&this->queue_mutex);
    	pthread_cond_destroy(&this->queue_not_empty);
    	pthread_cond_destroy(&this->queue_not_full);

   	}

};

class Socket {

public:

	int _server_fd;
	int _new_socket; 
	int _bound_socket;
	int _set_sock_opt;
	int _conn;
	int _request_response_code;
	struct sockaddr_in _address;
	socklen_t _addrlen;
	char _recv_buffer[BUFFER_SIZE] = {0};
	std::string _request_file = "";
	std::string __CLI__;
	std::string _request_file_buffer = "HTTP/1.1 404 OK\nContent-Type: " \
	    "text/plain\nContent-Length: 28\n\nLithon speed file not found!";
	std::vector<std::pair<std::string, std::string>> _GET_parameters;
	static std::string _GET_response;
	
	static std::string get_request_type(char* buffer) {
	
		std::string full_buffer_str(buffer);
   		std::istringstream iss(full_buffer_str);
    	std::string first_line;

    	if (std::getline(iss, first_line)) {

			size_t first = first_line.find(' ');
			if (first != std::string::npos) {
        		return first_line.erase(first);
    		}
			return "";	
    	}

		return "";
		
	}
			
	static void* worker_thread_task(void* socketDescriptor) {
	
    	int clientSocket = *(int*)socketDescriptor;
    	char buffer[1024] = {0};
    	recv(clientSocket, buffer, 1024, 0);

		std::string response = Socket::process_request(buffer);

    	send(clientSocket, response.c_str(), strlen(response.c_str()), 0);
    	close(clientSocket);
		return nullptr;
		
	}
			
	void start_many() {
		
		ThreadPool* thread_pool = new ThreadPool(5);

		int i = 0;
	
		while(true) {
		
    		sockaddr_in clientAddress;
        	socklen_t clientAddressLength = sizeof(clientAddress);

        	this->_new_socket = accept(
				this->_server_fd, 
				(sockaddr*)&clientAddress, 
				&clientAddressLength);
			
           if (this->_new_socket == -1) {
               std::cerr << "Error accepting connection" << std::endl;
               continue;
           }
			
			int _new_socket = this->_new_socket;
			
			if (this->__CLI__.find("threaded") > 0) {
			
				thread_pool->add_thread(
					worker_thread_task,((void*)&this->_new_socket));
				
			} else {
			
				thread_pool->add_task([_new_socket]() {
				
					worker_thread_task(((void*)&_new_socket));

				});
			}
			i++;
    	}
		delete thread_pool;
	}
	
	static void process_DELETE() {
	
		std::cout << "DELETING" << std::endl;
	
	};
	
	static void process_PUT() {
	
		std::cout << "PUTING" << std::endl;

	};
	
	static void process_FILE() {
	
		std::cout << "FILEING" << std::endl;
	
	};
	
	static std::string process_GET(char buffer[1024]) {
	
		std::cout << "GETING" << std::endl;
		std::cout << "Type:" <<  Socket::get_request_type(buffer) << std::endl;
		auto [url_map, query_string] = Socket::get_query_string_map(buffer);
		/***
		 *
		 * testing tuple return
		 *
		 */
		std::map<std::string,std::string>::iterator it;
		for (it = url_map.begin(); it != url_map.end(); ++it) {
            std::cout << "Key::: " << it->first << ", Val::: " << it->second;
 		    std::cout << std::endl;
        }
		std::cout << query_string << std::endl;
		std::string request_file = WEB_ROOT + Socket::get_request_data("url_path",buffer);
		std::string response = Socket::process_request_response(request_file);
		
		return response;
	};
	
	static std::string get_file_type(std::string file) {
	    std::cout << "GETTING FILE TYPE:: " << file.substr(file.rfind(".") + 1) << std::endl;
	    return file.substr(file.rfind(".") + 1);
	};
	
	static std::string get_file_name(std::string file) {
	    std::cout << "GETTING FILE NAME:: " << file.substr(file.rfind("/") + 1) << std::endl;
	    return file.substr(file.rfind("/") + 1);
	};
	
	static bool is_executable(std::string request_file) {
        // X_OK checks for execute permission
        return (access(request_file.c_str(), X_OK) == 0); 
    }
    
    static void system_exec_file() {
      
        std::string request_file = "../web/test/exc/speed.exc";
        std::cout << "FILE!!!!!" << request_file << std::endl;
        
        FILE* pipe = popen(request_file.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string result = "";
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
            
            std::string request_file_buffer = result;
	        request_file_buffer = "HTTP/1.1 200 OK\nContent-Type: " \
		        "text/plain\nContent-Length: " + 
		        std::to_string(request_file_buffer.length()) + 
		        "\n\n" + request_file_buffer;
        }  
    };
    
    static std::string get_response_string(std::string data) {
        std::string request_file_buffer = data;
	    request_file_buffer  =  "HTTP/1.1 200 OK\nContent-Type: " \
		                        "text/plain\nContent-Length: " + 
		                        std::to_string(request_file_buffer.length()) + 
		                        "\n\n" + request_file_buffer;
		    
		   return request_file_buffer;
    };
	
	static std::string process_file_exists(std::string request_file) {
	
	    std::cout << "Working execing script 1" << request_file << std::endl;
		std::ifstream file(request_file);
		if (file.good()) {
		    
		    std::cout << "Working execing script 2" << request_file << std::endl;
		    if (is_executable(request_file)) {
		        std::string file_name = get_file_name(request_file);
		        std::string file_type = get_file_type(request_file);
		        
		        std::cout << "Working execing script 3" << request_file << std::endl;
                std::string response = "Working on executing script";
                system_exec_file();
                
                return get_response_string(response);
		
		    }  else {
                std::stringstream buffer;
			    buffer << file.rdbuf();
			    return get_response_string(buffer.str());
    	    }
		}
		return "";
	};
	
	static std::string process_request_response(std::string request_file) {
		std::ifstream file(request_file);
		if (file.good()) {
			std::string response = process_file_exists(request_file);
			return response;
        } else {
            std::cout << "File not exists: " << request_file << std::endl;
			std::string response = "HTTP/1.1 200 OK\nContent-Type: " \
				"text/plain\nContent-Length: 28\n\nSpeed server file not found!";
			return response;
    	}
	};
	
	static std::string process_POST(char buffer[1024]) {
	
	    std::string response = buffer;
	
		std::cout << "POSTING" <<  response << std::endl;
	    
	    return response;
	};
	
	static std::string process_request(char buffer[1024]) {
		std::cout << "\n" << buffer << std::endl;
		std::string req_type = Socket::get_request_type(buffer);
		if (req_type == "GET") {
			return Socket::process_GET(buffer);
		}
		if (req_type == "POST") {
			return Socket::process_POST(buffer);		
		}
		if (req_type == "PUT") {
			Socket::process_PUT();
		}
		if (req_type == "DELETE") {
			Socket::process_DELETE();
		}
		return "Not found";
	};
	
	static std::tuple<std::map<std::string, std::string>, std::string>  
	    get_query_string_map(char buffer[1024]) {
	
		std::string request(buffer), request_line, method, url_path, http_version;
    	std::istringstream iss(request);
    	std::getline(iss, request_line);
    	std::istringstream iss_line(request_line);
    	iss_line >> method >> url_path >> http_version;
		std::cout << "Url path: " <<  url_path << std::endl;
		std::map<std::string, std::string> url_map;
		std::string query_string = url_path;
		int question = query_string.find("?");
		int equals = query_string.find("=");
		std::vector<std::string> _GET_structure;
		
		if (question != std::string::npos && equals != std::string::npos) {
			
			size_t start_pos = query_string.find_first_of("?");
			query_string = query_string.substr(
			    start_pos, std::string::npos);
			query_string = query_string.substr(1);
			std::stringstream ss(query_string);
			std::string pair;
			int i = 0;
				
			while (std::getline(ss, pair, '&')) {
        		size_t pos = pair.find('=');
        		if (pos != std::string::npos) {
           		std::string key = pair.substr(0, pos);
            		std::string value = pair.substr(pos + 1);
					url_map.insert({key,value});
					i++;
        		}
    		}
		}
		return {url_map, query_string};
	}
	
	static std::string get_request_data(std::string name,char buffer[1024]) {
		
    	std::string request(buffer);
    	std::istringstream iss(request);
    	std::string request_line;
    	std::getline(iss, request_line);
    	std::istringstream iss_line(request_line);
    	std::string method, url_path, http_version;
    	iss_line >> method >> url_path >> http_version;

		if (name == "url_path") {			
			char target_char = '?';
    		size_t pos = url_path.find(target_char);
    		if (pos != std::string::npos) {
        		url_path = url_path.erase(pos);
    		}
			return url_path;
		}
		if (name == "method") {
			return method;
		}
		if (name == "http_version") {
			return http_version;
		}
    	std::string data;
    	std::string line;
    	while (std::getline(iss, line)) {
        	if (line.rfind(name, 0) == 0) {
            	data = line.substr(name.length());
            	size_t start_pos = data.find_first_not_of(" \t");
            	if (start_pos != std::string::npos) {
                 data = data.substr(start_pos);
            	}
            	size_t end_pos = data.find_first_of(" \t\r\n");
            	if(end_pos != std::string::npos) {
               	data = data.substr(0, end_pos);
            	}
            	return data;
        	}
    	}
    	return "";
	};
	
	bool is_socket_valid(int sockfd) {
  		if (fcntl(sockfd, F_GETFD) == -1) {
    		if (errno == EBADF) {
      			return false;
    		} else {
      			throw std::system_error(errno, std::generic_category(), "fcntl");
    		}
  		}
  		return true;
	};
	
	void start(std::string url = "") {

		char buffer[BUFFER_SIZE] = {0};
		// Accepting and handling connections
		this->_new_socket = accept(
			this->_server_fd, 
			(struct sockaddr *)&this->_address, 
			(socklen_t*)&this->_addrlen);
    	if (this->_conn < 0) {
       		perror("accept");    
        	exit(EXIT_FAILURE);
		}
		worker_thread_task(((void*)&this->_new_socket));
	};

	void kill() {
		close(this->_new_socket);
    	close(this->_server_fd);
	};
	
	void make() {

		int bound_socket, set_sock_opt, opt = 2;
		this->_addrlen = sizeof(this->_address);
    	this->_server_fd = (socket(AF_INET, SOCK_STREAM, 0));
    	if (this->_server_fd == -1) {
			std::cerr << "Error creating socket" << std::endl;
			return;
	    }
		// Forcefully attaching socket to the port 8080
		try{
			// Verify server file descriptor
			bool socket_valid = is_socket_valid(this->_server_fd);
			set_sock_opt = setsockopt(
				this->_server_fd, 
				SOL_SOCKET, 
				SO_REUSEADDR, 
				&opt, 
				sizeof(opt));
		} catch (const std::system_error e) {
        	std::cerr << "Error: Invalid argument - " << e.what() << std::endl;
		}
		if (set_sock_opt) {
        	perror("setsockopt");
        	exit(EXIT_FAILURE);
    	}
		// set sockaddr_in structure
		this->_address.sin_family = AF_INET;
		this->_address.sin_addr.s_addr = INADDR_ANY;
		this->_address.sin_port = htons(PORT_ONE);
		this->_address.sin_addr.s_addr = inet_addr("127.0.0.1");
		std::cout << "Server listening on port: " << PORT_ONE<< std::endl;
		// Binding the socket to the address
		bound_socket = bind(
			this->_server_fd, 
			(struct sockaddr *)&this->_address, 
			sizeof(this->_address));
    	if (bound_socket < 0) {
        	perror("bind failed");
        	exit(EXIT_FAILURE);
    	}
		std::cout << "Server file descriptor: " << this->_server_fd << std::endl;
		// Listening for connections
		this->_conn = listen(this->_server_fd, 3);
		if (this->_conn < 0) {
			perror("listen failed ooff");
			exit(EXIT_FAILURE);
		}
	}
};

class Server {

public:

	std::string __CLI__;
	Socket* _socket;
	
	void make() {
		this->_socket = new Socket();
		this->_socket->make();
    };

	void start_many() {
		this->_socket->__CLI__ = this->__CLI__;
		this->_socket->start_many();
    };

	void start() {
		this->_socket->start();
	};

    void stop() {
		this->_socket->kill();
		delete this->_socket;
    };
};
#endif