#ifndef PROCESS
#define PROCESS

#include "classes.cpp"

int main(int argc, char* argv[]) {
	std::cout << "FRESH COMPILE" << std::endl;
	Server* server = new Server();
	server->make();
	if (argc == 3 && strcmp(argv[2],"pooled") == 0) {
		server->__CLI__ = "pooled";
	}
	if (argc == 3 && strcmp(argv[2],"threaded") == 0) {
		server->__CLI__ = "threaded";
	}
	if (argc == 3 && strcmp(argv[1],"one") == 0) {
		server->start();
	}
	if (argc == 3 && strcmp(argv[1],"many") == 0) {
		server->start_many();
	}
	server->stop();
	delete server;
    return 0;
}
#endif