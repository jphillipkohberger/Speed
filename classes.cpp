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
#include "nlohmann/json.hpp"
#define PORT_ONE 8080
#define BUFFER_SIZE 1024
#define WEB_ROOT "../web" //relative to this file
#define THREAD_POOL_SIZE 5
#define TASK_COUNT 10

using json = nlohmann::json;

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
		
	static void print(char buffer[1024]) {
	    std::cout << "\n\n" << buffer << "\n\n" << std::endl;
	};		
		
	static void print(std::string str) {
	    std::cout << "\n\n" + str + "\n\n" << std::endl;
	};	
			
	static void* worker_thread_task(void* socketDescriptor) {
    	int clientSocket = *(int*)socketDescriptor;
    	char buffer[1024] = {0};
    	recv(clientSocket, buffer, 1024, 0);
		std::string response = Socket::process_request(buffer);
    	send(clientSocket, response.c_str(), strlen(response.c_str()), 0);
    	close(clientSocket);
		return nullptr;
	};
			
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
	
		std::cout << "PUTTING" << std::endl;

	};
	
	static void process_FILE() {
	
		std::cout << "FILING" << std::endl;
	
	};
	
	static std::string process_GET(char buffer[1024]) {
	
		std::cout << "GETTING" << std::endl;
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
		
		print(buffer);
		
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
	
	static std::string get_post_data_type(std::string data_string) {

        int equals = data_string.find("=");
        int amper = data_string.find("&");
         if (equals != std::string::npos && amper != std::string::npos) {  
            return "FORM";        
        }
	    
	    try {
	        std::string data_str_frst = data_string;
	        json parsed_json = json::parse(data_str_frst);
	        if(parsed_json.size() > 0){
	            return "JSON";
	        }
        } catch (const json::parse_error& e) {
        }
	    return "NONE";
	}
	
	static std::string process_POST(char buffer[1024]) {
	    std::string data_string = get_request_data("post_data", buffer);
	    std::string data_string_type = get_post_data_type(data_string);
	    auto [url_map, qstr] = split_query_string_map(data_string);
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
	    std::string response = buffer;
	    return response;
	};
	
	static std::string process_request(char buffer[1024]) {
		// std::cout << "\n\nBuffer:\n\n" << buffer << "\n\n" <<std::endl;
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
	    get_json_from_query_string(std::string query_string) {
        std::map<std::string, std::string> url_map;
        try {
            json parsed_json = json::parse(query_string);
	        if (parsed_json.is_object()) {
                for (const auto& item : parsed_json.items()) {
                    auto value = item.value();
                    auto key = item.key();
                    url_map.insert({key,value});
                }
	        }
	        return {url_map, query_string};
        } catch (const json::parse_error& e) {
            return {url_map, query_string}; 
        }
        return {url_map, query_string};
	}
	
	static std::string remove_question_from_query_string(std::string query_string) {
	    size_t start_pos = query_string.find_first_of("?");
		if (start_pos == std::string::npos) {
			start_pos = 0;
		} else {
		    start_pos = 1;
		}
		query_string = query_string.substr(
			start_pos, std::string::npos);
		return query_string;
	}
	
	static std::map<std::string, std::string> split_query(std::string query_string) {
	    std::map<std::string, std::string> url_map;
	    std::stringstream ss(remove_question_from_query_string(query_string));
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
		return url_map;
	}
	
	static std::tuple<std::map<std::string, std::string>, std::string>  
	    split_query_string_map(std::string query_string) {
	    if (get_post_data_type(query_string) == "JSON") 
	        return get_json_from_query_string(query_string);
	    return {split_query(query_string), query_string};  
    };     
	
	static std::tuple<std::map<std::string, std::string>, std::string>  
	    get_query_string_map(char buffer[1024]) {
	    RequestVariables rv = get_request_variables(buffer);
		std::string request(buffer);
		std::map<std::string, std::string> url_map;
		std::string query_string =  rv.url_path;
		int question = query_string.find("?");
		int equals = query_string.find("=");
		if (question != std::string::npos && equals != std::string::npos) {	
    		auto [rl_map, qstr] = split_query_string_map(query_string);
    		url_map = rl_map;
    		query_string = qstr;
		}
		return {url_map,query_string};
	};
	
	struct  RequestVariables {
	    std::istringstream iss;
	    std::istringstream iss_line;
	    std::string method;
	    std::string url_path;
	    std::string http_version;
	};
	
	static RequestVariables get_request_variables(char buffer[1024]) {
	    RequestVariables request_variables;
	    std::string request(buffer);
    	request_variables.iss.str(request);
    	std::string request_line;
    	std::getline(request_variables.iss, request_line);
    	request_variables.iss_line.str(request_line);
    	std::string method, url_path, http_version;
    	request_variables.iss_line >> request_variables.method >> request_variables.url_path >> request_variables.http_version;
    	return request_variables;
	};
	
	static std::string is_question_query_string(std::string url_path){
		char target_char = '?';
		size_t pos = url_path.find(target_char);
		if (pos != std::string::npos) {
    		url_path = url_path.erase(pos);
		}
		return url_path;
	};
	
	static std::string get_request_body_data(RequestVariables* rv) {
	    std::string line;
    	int current, previous = -1;
    	while (std::getline(rv->iss, line)) {
        	size_t end_pos = line.find_last_not_of(" \t\n\r\f\v");
            if (std::string::npos == end_pos) current = 1;
            if (previous != -1 && previous== 1) return line;
            previous = current;
    	}
    	return "";
	};
	
	static std::string get_request_data(std::string name,char buffer[1024]) {
    	RequestVariables rv = get_request_variables(buffer);
		if (name == "url_path") return is_question_query_string(rv.url_path);
		if (name == "method") return rv.method;
		if (name == "http_version") return rv.http_version;
		if (name == "post_data") return get_request_body_data(&rv);
    	return "";
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
	
	void create_socket() {
		this->_addrlen = sizeof(this->_address);
    	this->_server_fd = (socket(AF_INET, SOCK_STREAM, 0));
    	if (this->_server_fd == -1) {
			std::cerr << "Error creating socket" << std::endl;
			return;
	    }
	};
	
	void set_socket_option() {
	    int set_sock_opt, opt = 2;
		try{
			set_sock_opt = setsockopt(this->_server_fd, SOL_SOCKET, SO_REUSEADDR, 
				&opt, sizeof(opt));
		} catch (const std::system_error e) {
        	std::cerr << "Error: Invalid argument - " << e.what() << std::endl;
		}
		if (set_sock_opt) {
        	perror("setsockopt");
        	exit(EXIT_FAILURE);
    	}
	};
	
	void set_socket_address(){
	    this->_address.sin_family = AF_INET;
		this->_address.sin_addr.s_addr = INADDR_ANY;
		this->_address.sin_port = htons(PORT_ONE);
		this->_address.sin_addr.s_addr = inet_addr("127.0.0.1");
		std::cout << "Server listening on port: " << PORT_ONE<< std::endl;
	};
	
	void bind_socket_to_address() {
	    int bound_socket = bind(
			this->_server_fd, 
			(struct sockaddr *)&this->_address, 
			sizeof(this->_address));
    	if (bound_socket < 0) {
        	perror("bind failed");
        	exit(EXIT_FAILURE);
    	}
		std::cout << "Server file descriptor: " << this->_server_fd << std::endl;
	};
	
	void listen_for_connections() {
		this->_conn = listen(this->_server_fd, 3);
		if (this->_conn < 0) {
			perror("listen failed");
			exit(EXIT_FAILURE);
		}
	};
	
	void make() {
	    this->create_socket();
    	this->set_socket_option();
		this->set_socket_address();
		this->bind_socket_to_address();
		this->listen_for_connections();
	};
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