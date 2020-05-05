#include <string.h>
#include <stdlib.h>
#include "client.h"
#include "translator.h"

#define MAX_ARG_LEN 30 //max largo de los argv
#define SERVER_RESPONSE_LEN 3 //"OK\n"

int client_create(client_t *self, int argc, char const *argv[]){
	file_reader_t file_reader;
	socket_t client_socket;
	char host_name[MAX_ARG_LEN], service[MAX_ARG_LEN];
	int max_length = MAX_ARG_LEN;

	file_reader_create(&file_reader, argc, argv[3]);
	socket_create(&client_socket);

	strncpy(host_name, argv[1], max_length);
	strncpy(service, argv[2], max_length);
	
	self->file_reader = file_reader;
	self->client_socket = client_socket;

	socket_connect(&(self->client_socket), host_name, service);

	return 0;
}

int client_run(client_t *self){	
	int id = 1;
	char response[SERVER_RESPONSE_LEN+1]="";

	translator_t translator;
	translator_create(&translator);
	
    while (file_reader_status(&(self->file_reader))){
    	file_reader_read_line(&(self->file_reader));
		translator_make_message(&translator, &(self->file_reader), id);				
		socket_send(&(self->client_socket), translator.header, translator.header_len);
		socket_send(&(self->client_socket), translator.body, translator.body_len);
		socket_receive(&(self->client_socket), response, SERVER_RESPONSE_LEN);
		_client_show(response, id);
		file_reader_free(&(self->file_reader));
		translator_free(&translator);
		id++;
	}

	translator_destroy(&translator);
	return 0;
}

int _client_show(char* response, int id){
	printf("0x%08x: %s", id, response);

	return 0;
}


int client_destroy(client_t *self){
	socket_shutdown(&(self->client_socket), 2);
	socket_destroy(&(self->client_socket));
	file_reader_destroy(&(self->file_reader));

	return 0;
}
