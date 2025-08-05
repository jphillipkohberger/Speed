#ifndef PROCESS
#define PROCESS

#include <cstdlib>
#include <iostream>

//./server many pool &>/dev/null &
// curl --http0.9 -d "foo=bar&bar=foo" -H "Content-Type: application/x-www-form-urlencoded" -X POST http://localhost:8080/test/html/index.html
// main.cpp -o server && ./server many pooled & curl --http0.9 -d
// "foo=bar&bar=foo" -H "Content-Type: application/x-www-form-urlencoded" -X
// POST http://127.0.0.1:8080/test/html/index.html
// curl --http0.9 -X POST http://127.0.0.1:8080/test/html/index.html -H
// 'Content-Type: application/json' -d '{"name":"Leo","age":26}'
/***
 * CLI - Need functions for compiling various files, executing
 * various files, automatic compilation on file save
 ***/

class CLI {

public:
  char **args;
  int arg_count;
  void set_args(char *args[]) {
    std::cout << "Number of arguments (argc): " << this->arg_count << std::endl;
    ;
    for (int i = 0; i < this->arg_count; ++i) {
      std::cout << "Argument " << i << ": " << this->args[i] << std::endl;
    }
  }

  void build() {
    int returnCode = system("g++ -std=c++17 main.cpp -o server");
    // int returnCode = 3;//system("echo hey there");
    if (returnCode == 0) {
      std::cout << "Built successfully" << std::endl;
    } else {
      std::cout << "Build failed: " << returnCode << std::endl;
    }
  }
};

// g++ -std=c++17 main.cpp -o server && echo "completed"
// g++ -std=c++17 build.cpp -o build && echo "completed"
// wget --post-data "user=fo&word=bar"
// http://127.00.0.1:8080/test/html/index.html &> /dev/null wget
// http://127.00.0.1:8080/test/html/index.html &> /dev/null
int main(int argc, char *argv[]) {
  CLI *cli = new CLI();
  cli->args = argv;
  cli->arg_count = argc;
  cli->set_args(argv);
  cli->build();
  delete cli;
  return 0;
}

#endif