#include <stdio.h>
#include <string.h>
#include "socket.h"
#include "server.h"

int main(int argc, char const *argv[]){	
	server_t server;

	server_create(&server, argv[1]);
	server_run(&server);
	server_destroy(&server);
}
