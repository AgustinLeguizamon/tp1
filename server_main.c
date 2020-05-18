#include <stdio.h>
#include <string.h>
#include "common_socket.h"
#include "server_server.h"

#define ERROR -1

int main(int argc, char const *argv[]){	
	server_t server;

	server_create(&server, argv[1]);
	if(server_run(&server) == ERROR){
		printf("Error in server_run\n");
		return ERROR;
	}
	server_destroy(&server);

	return 0;
}
