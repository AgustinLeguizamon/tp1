#include <stdio.h>

#include "client_client.h"

int main(int argc, char const *argv[]){
	client_t client;

	client_create(&client, argc, argv);
	client_run(&client);
	client_destroy(&client);

	return 0;
}
	
