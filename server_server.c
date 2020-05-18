#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "server_server.h"


#define HEADER_SIGNATURE_LEN 16
#define ARRAY_WITH_ARGUMENTS 5
#define ARRAY_WO_ARGUMENTS 4
#define PATH 1
#define DESTINY 6
#define INTERFACE 2
#define METHOD 3
#define SIGNATURE 8
#define HEADER_SIGNATURE_UINT32_N 3
#define ERROR -1



/*Lee los primeros 16 bytes del header, guardando los 3 Uint32
en un array.
*/
static int _server_read_header_signature(char** cursor, 
	uint32_t header_info[]);

/* Recorre todo el array del header con un cursor deteniendose 
en cada tipo de parametro y guardando su string en una matriz
*/

static int _server_read_header_array(uint32_t header_info[], 
	char* header_buffer, char* body_buffer, int body_len);

/*Lee el string de uno del parametro al que este apuntando el
cursor del header y lo guarda en input_string*/

static char* _server_read_option(char** cursor, char* word);

/*Lee el uint32 del parametro firma 
correspondiente a la cantidad de argumentos del cuerpo */

static int _server_read_signature(char* cursor);


/*Imprime por pantalla los datos que recibido por el socket
*/
static int _server_show(uint32_t header_info[], char* body_buffer, 
	char* destiny, char* path, char* interface, char* method, int n_arguments);

/*Imprime por pantalla los parametros recibidos por el socket
*/
static int _server_show_parameters(char* body_buffer, int n_arguments);

/*Envia el mensaje "OK\n* al client*/
static int _server_send_response(server_t *self);

static int _server_error_check(char* heap_array);

int server_create(server_t *self, char const* argv){
	socket_t acep_socket;
	socket_t server_socket;
	
	socket_create(&acep_socket);
	socket_create(&server_socket);

	self->acep_socket = acep_socket;
	self->server_socket = server_socket;
	
	if(socket_bind_and_listen(&(self->acep_socket), argv)){
		return ERROR;
	}
	
	socket_accept(&(self->acep_socket), &(self->server_socket));

	return 0;
}

int server_run(server_t *self){	
	int socket_state = 1;
	
	while(socket_state > 0){
		//este lo defino de manera estatica porque 
		//la firma del header siempre tiene el mismo largo
		char header_signature_buffer[HEADER_SIGNATURE_LEN];
		socket_state = socket_receive(&(self->server_socket),
			header_signature_buffer, HEADER_SIGNATURE_LEN);

		if(socket_state == ERROR){
			return ERROR;
		}

		if(socket_state > 0){
			char* cursor = header_signature_buffer;
			
			//el header siempre tiene 3 uint32_t
			uint32_t header_info[HEADER_SIGNATURE_UINT32_N];
			_server_read_header_signature(&cursor, header_info);
			
			//el +1 es por el ultimo byte de padding
			//header_info[2] = uint32_n long array
			char* header_buffer = malloc(header_info[2]+1);
			if(header_buffer == NULL){
				printf("header_buffer malloc failed\n");
				return ERROR;
			}

			memset(header_buffer,0,(header_info[2]+1));
			if(socket_receive(&(self->server_socket), header_buffer, 
					(header_info[2]+1))==ERROR){
				return ERROR;
			}
			uint32_t body_len = header_info[0];
			char* body_buffer = malloc(body_len);
			if(body_buffer == NULL){
				free(header_buffer);
				printf("body_buffer malloc failed\n");
				return ERROR;
			}
			if(socket_receive(&(self->server_socket), body_buffer, body_len)==ERROR){
				return ERROR;	
			}
			if(_server_read_header_array(header_info, header_buffer, 
					body_buffer, body_len)==ERROR){
				return ERROR;
			}
			if(_server_send_response(self)==ERROR){
				return ERROR;
			}
			free(header_buffer);
			free(body_buffer);
		}
	}
	return 0;
}


int _server_read_header_signature(char** cursor, uint32_t header_info[]){
	if(**cursor != 'l'){
		printf("Error reading header signature\n");
		return ERROR;
	} else {
		for (int i = 0; i < 3; ++i){
			*cursor += sizeof(uint32_t);
			uint32_t len = *((uint32_t*) (*cursor));
			header_info[i]=len;
		}
	}

	return 0;
}

int _server_read_header_array(uint32_t header_info[], char* header_buffer, 
		char* body_buffer, int body_len){
	int arguments_amount = 0;
	
	char* cursor = header_buffer;

	int options_to_read;
	if(body_len > 0){
		options_to_read = ARRAY_WITH_ARGUMENTS;
	} else {
		options_to_read = ARRAY_WO_ARGUMENTS;
	}
	
	char* destiny = malloc(1); 
	char* path = malloc(1); 
	char* interface= malloc(1); 
	char* method= malloc(1);
	
	int error = _server_error_check(destiny) + _server_error_check(path) + 
		_server_error_check(interface) + _server_error_check(method);
	
	int i=0;
	while (i < options_to_read && error == 0){
		if (*cursor != 0){
			int option = *cursor;
			switch(option){
				case PATH:
					path = _server_read_option(&cursor, path);
					error = _server_error_check(path);
					break;
				case DESTINY:
					destiny = _server_read_option(&cursor, destiny);
					error = _server_error_check(path);
					break;
				case INTERFACE:
					interface = _server_read_option(&cursor, interface);
					error = _server_error_check(path);
					break;
				case METHOD:
					method = _server_read_option(&cursor, method);
					error = _server_error_check(path);
					break;
				case SIGNATURE:
					arguments_amount = _server_read_signature(cursor);
					break;
			}
			i++;
		} else {
			cursor++;
		}
	}

	if(error != 0){
		free(destiny);
		free(path);
		free(interface);
		free(method);
		return ERROR;
	}

	_server_show(header_info, body_buffer, destiny, path, 
		interface, method, arguments_amount);

	free(destiny);
	free(path);
	free(interface);
	free(method);

	return 0;
}

char* _server_read_option(char** cursor, char* word){
	(*cursor) += sizeof(uint32_t); //cursor apunta a long
	uint32_t option_len = *((uint32_t*) (*cursor));
	word = realloc(word, option_len+1);
	if(word == NULL){
		printf("realloc failed\n");
		return NULL;
	}
	memset(word, 0, option_len+1);
					
	(*cursor) += sizeof(uint32_t); //cursor apunta array
	for (int i = 0; i < option_len+1; ++i){
		word[i]=(**cursor);
		(*cursor)++;
	}

	return word;
}

int _server_show(uint32_t header_info[], char* body_buffer, 
		char* destiny, char* path, char* interface, char* method, int n_arguments){
	printf("* Id: 0x%08x\n", header_info[1]);
	printf("* Destino: %s\n", destiny);
	printf("* Ruta: %s\n", path);
	printf("* Interfaz: %s\n", interface);
	printf("* Metodo: %s\n", method);
	if(n_arguments > 0){
		_server_show_parameters(body_buffer, n_arguments);		
	}
	printf("\n");

	return 0;
}

int _server_show_parameters(char* body_buffer, int n_arguments){
	char* argument = malloc(1);		
	if(argument == NULL){
		printf("malloc failed\n");	
		return ERROR;
	}

	int j=0;
	printf("* Parametros:\n");
	char* cursor = body_buffer;
	do{		
		uint32_t arg_len = *((uint32_t*)cursor);
		argument = realloc(argument, arg_len+1);
		if(argument == NULL){
			printf("realloc failed\n");
			free(argument);
			return ERROR;
		}
		memset(argument,0,arg_len+1);
		cursor += sizeof(uint32_t);
		for (int i = 0; i < arg_len; i++){
			argument[i] = *cursor;
			cursor++;
		}
		cursor++;
		printf("    * %s\n", argument);
		j++;
	} while (j < n_arguments);
	
	free(argument);
	return 0;
}

int _server_read_signature(char* cursor){
	(cursor) += sizeof(uint32_t);
	int n_arg = *cursor;
	return n_arg;
}
					

int _server_send_response(server_t *self){
	char* response = "OK\n";

	if(socket_send(&(self->server_socket), response, strlen(response))==ERROR){
		printf("Error sending server response to client\n");
		return ERROR;
	}

	return 0;
}


int _server_error_check(char* p_alloc){
	if(p_alloc == NULL){
		return ERROR;
	}
	return 0;
}


int server_destroy(server_t *self){
	socket_shutdown(&(self->server_socket), 2);
	socket_destroy(&(self->acep_socket));
	socket_destroy(&(self->server_socket));

	return 0;
}
