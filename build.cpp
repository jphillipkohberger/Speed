#ifndef PROCESS
#define PROCESS

#include <cstdlib>
#include <iostream>

//./server many pool &>/dev/null &
//curl --http0.9 -d "foo=bar&bar=foo" -H "Content-Type: application/x-www-form-urlencoded" -X POST http://localhost:8080/test/html/index.html

/***
 * CLI - Need functions for compiling various files, executing
 * various files, automatic compilation on file save
 ***/

class CLI {

public:

	char** args;
	int arg_count;
	
	CLI(char** args) {
	
		this->arg_count = sizeof(args) - 1;
		this->args = args;
		
		std::cout << "Number of arguments (argc): " << this->arg_count;
		std::cout << std::endl;
    	std::cout << "Program name (argv[0]): " << this->args[0] << std::endl;

    	if (this->arg_count > 1) {
       		std::cout << "Additional arguments:" << std::endl;
        	for (int i = 1; i < this->arg_count; ++i) {
           	std::cout << "args[" << i << "]: " << this->args[i] << std::endl;
        	}
    	}
	
	}
	
	void build() {
		
		std::cout << "Building project 2:" << std::endl;
		
		int returnCode = system("g++ -std=c++17 main.cpp -o server");
		
		if (returnCode == 0) {
       		
			std::cout << "Built successfully" << std::endl;
    	
		} else {
       		
			std::cout << "Build failed: " << returnCode << std::endl;
    	
		}
		
	}

};

// g++ -std=c++17 main.cpp -o server && echo "completed"
// g++ -std=c++17 build.cpp -o build && echo "completed"
// wget --post-data "user=fo&word=bar" http://127.00.0.1:8080/test/html/index.html &> /dev/null
// wget http://127.00.0.1:8080/test/html/index.html &> /dev/null
int main(int argc, char* argv[]) {

	CLI* cli = new CLI(argv);
	
	cli->build();
	
    return 0;

}

#endif