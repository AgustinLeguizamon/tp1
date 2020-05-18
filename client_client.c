#include <string.h>
#include <stdlib.h>
#include "client_client.h"
#include "client_translator.h"

#define ERROR -1
#define SERVER_RESPONSE_LEN 3 //"OK\n"

/*Muestra la respuesta del servidor con el numero de
@param id
*/
static int _client_show(char* response, int id);


int client_create(client_t *self, int argc, char const *argv[]){
	file_reader_t file_reader;
	socket_t client_socket;

	if(file_reader_create(&file_reader, argc, argv[3]) == ERROR){
		return ERROR;
	}
	socket_create(&client_socket);

	/*Si no hago esta copia el compilador me 
	tira warning de variable sin utilizar*/
	self->file_reader = file_reader;
	self->client_socket = client_socket;

	if(socket_connect(&(self->client_socket), argv[1], argv[2]) == ERROR){
		return ERROR;
	}

	return 0;
}

int client_run(client_t *self){	
	translator_t translator;
	translator_create(&translator);
	
	char response[SERVER_RESPONSE_LEN+1]="";
	int id = 1;
    while (file_reader_status(&(self->file_reader))){
    	file_reader_read_line(&(self->file_reader));
		translator_make_message(&translator, &(self->file_reader), id);				
		if(socket_send(&(self->client_socket), translator.header, 
				translator.header_len) == ERROR){
			return ERROR;
		}
		if(socket_send(&(self->client_socket), translator.body, 
				translator.body_len) == ERROR){
			return ERROR;
		}
		if(socket_receive(&(self->client_socket), response, 
				SERVER_RESPONSE_LEN) == ERROR){
			return ERROR;
		}
		_client_show(response, id);
		file_reader_free_input(&(self->file_reader));
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
	socket_destroy(&(self->client_socket));
	file_reader_destroy(&(self->file_reader));

	return 0;
}
