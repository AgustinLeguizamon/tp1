#include <stdio.h>

#include "client_client.h"

#define ERROR -1

int main(int argc, char const *argv[]){
	client_t client;

	if(client_create(&client, argc, argv) == ERROR){
		printf("client_create error\n");
		return ERROR;
	}
	if(client_run(&client) == ERROR){
		printf("client_run error\n");
		return ERROR;
	}
	client_destroy(&client);

	return 0;
}
	
