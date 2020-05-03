#include <string.h>
#include <stdlib.h>
#include "client.h"
#include "translator.h"

#define MAX_ARG_LEN 10 //max largo de los argv
#define SERVER_RESPONSE_LEN 3 //OK

void client_create(client_t *self, int argc, char const *argv[]){
	file_reader_t file_reader;
	socket_t client_socket;
	char host_name[MAX_ARG_LEN], service[MAX_ARG_LEN];
	int max_length = MAX_ARG_LEN;

	self->file_reader = file_reader;
	self->client_socket = client_socket;

	strncpy(host_name, argv[1], max_length);
	strncpy(service, argv[2], max_length);

	file_reader_create(&(self->file_reader), argc, argv[3]);
	socket_create(&(self->client_socket));
	socket_connect(&(self->client_socket), host_name, service);
}

void client_run(client_t *self){	
	int id = 1;
	int file_status = 0;
	char* input_line;
	char response[SERVER_RESPONSE_LEN+1]="";

    while (file_status != EOF){
    	input_line = file_reader_read_file(&(self->file_reader), &file_status);
		message_t message = translator_dbus(input_line, id);
				
		socket_send(&(self->client_socket), message.header, message.header_len);
		socket_send(&(self->client_socket), message.body, message.body_len);
		socket_receive(&(self->client_socket), response, SERVER_RESPONSE_LEN);
		_client_show(response, id);
		free(input_line);
		free(message.header);
		free(message.body);
		id++;
	}
}

int _client_show(char* response, int id){
	printf("0x%08x: %s\n", id, response);

	return 0;
}


void client_destroy(client_t *self){
	socket_shutdown(&(self->client_socket), 2);
	socket_destroy(&(self->client_socket));
	file_reader_destroy(&(self->file_reader));
}
