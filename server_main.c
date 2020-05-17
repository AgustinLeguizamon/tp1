#include <stdio.h>
#include <string.h>
#include "common_socket.h"
#include "server_server.h"

int main(int argc, char const *argv[]){	
	server_t server;

	server_create(&server, argv[1]);
	server_run(&server);
	server_destroy(&server);

	return 0;
}
