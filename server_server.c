#include <string.h>
#include <stdio.h>
#include "server.h"

#define MAX_ARG_SIZE 10
#define MAX_ARG_AMOUNT 32
#define HEADER_SIGNATURE_LEN 16
#define MAX_LINE_SIZE 250
#define ARRAY_WITH_ARGUMENTS 5
#define ARRAY_WO_ARGUMENTS 4

#define HEADER_SIGNATURE_UINT32_N 3

void server_create(server_t *self, char const* argv){
	int max_arg_size = MAX_ARG_SIZE;
	char service[MAX_ARG_SIZE];	
	socket_t acep_socket;
	socket_t server_socket;

	self->acep_socket = acep_socket;
	self->server_socket = server_socket;

	strncpy(service, argv, max_arg_size);
	
	socket_create(&(self->acep_socket));
	socket_bind_and_listen(&(self->acep_socket), service);
	socket_accept(&(self->acep_socket), &(self->server_socket));
}

void server_run(server_t *self){
	char buffer[MAX_LINE_SIZE];
	uint32_t header_info[HEADER_SIGNATURE_UINT32_N], body_len;
	char array_info[ARRAY_WO_ARGUMENTS][BUFF_SIZE];
	char body_info[MAX_ARG_AMOUNT][BUFF_SIZE];
	int n_arguments, socket_state = 1;
	char* cursor; 
	
	//while (recibir 3 mensaje:header signature, header y body)
	/*recibo el header 16 y luego el resto 119 + 1 padding */
	//int j=1;
	while(socket_state > 0){
	/*inicializo matriz*/
		for (int i = 0; i < MAX_ARG_SIZE; ++i){
			memset(array_info[i],0,sizeof(array_info[i]));
		}
		for (int i = 0; i < MAX_ARG_AMOUNT; ++i){
			memset(body_info[i],0,sizeof(body_info[i]));
		}

		socket_state = socket_receive(&(self->server_socket), 
			buffer, HEADER_SIGNATURE_LEN);
		printf("SOCKET: %d\n", socket_state);
		if(socket_state > 0){
			cursor = buffer;
			_server_read_header_signature(&cursor, header_info);
			
		printf("HEADER\n");
		for (int i = 0; i < 10; i++){
			printf("%02x ", buffer[i]);
		} //TEST

			socket_receive(&(self->server_socket), buffer, (header_info[2]+1));	
			n_arguments = _server_read_header(buffer, header_info, array_info);
			body_len = header_info[0];
			socket_receive(&(self->server_socket), buffer, body_len);
			
			if(n_arguments > 0){
				_server_read_body(buffer, body_info, n_arguments);
			}

			_server_show(header_info, array_info, body_info, n_arguments);
			_server_response(self);
		//	j++;
		}
	}
}

int _server_read_header(char* buffer, 
		uint32_t header_info[], char array_info[][BUFF_SIZE]){
	char* cursor = buffer;
	int n_arguments = 0;
	int body_len = header_info[0];

	for (int i = 0; i < 3; ++i){
		printf("header info: %u\n", header_info[i]);
	}

	n_arguments = _server_read_header_array(&cursor, array_info, body_len);

	return n_arguments;
}

int _server_read_body(char* buffer, 
		char body_info[][BUFF_SIZE], int n_arguments){
	int i = 0;
	char* cursor = buffer;

	printf("BODY\n");
	for (int i = 0; i < 10; i++){
		printf("%02x ", buffer[i]);
	} //TEST
	printf("\n ");

	while(i < n_arguments){
		cursor = _server_read_one_argument(cursor, body_info[i]);	
		i++;
		cursor++; //salto el /0
	}

	return 0;
}

char* _server_read_one_argument(char* cursor, char body_argument[]){
	uint32_t arg_len = *((uint32_t*)cursor);
	printf("arg_len:%u\n", arg_len);
	cursor += sizeof(uint32_t);
		for (int i = 0; i < arg_len; i++){
			body_argument[i] = *cursor;
			cursor++;
		}
	
	return cursor;
}

int _server_read_header_signature(char** cursor, uint32_t header_info[]){
	int ret = 0;
	uint32_t len;

	printf("READING HEADER SIGNATURE\n");

	if(**cursor != 'l'){
		ret =-1;
	} else {
		for (int i = 0; i < 3; ++i){
			*cursor += sizeof(uint32_t);
			len = *((uint32_t*) (*cursor));
			header_info[i]=len;
		}
	}

	return ret;
}

int _server_read_header_array(char** cursor, 
		char array_info[][BUFF_SIZE], int body_len){
	int option, arguments_amount = 0, i=0;
	int options_to_read;

	if(body_len > 0){
		options_to_read = ARRAY_WITH_ARGUMENTS;
	} else {
		options_to_read = ARRAY_WO_ARGUMENTS;
	}

	while (i < options_to_read){
		if (**cursor != 0){
			option = **cursor;
			printf("option: %d\n", option);
			switch(option){
				case 1:
					printf("PATH \n");
					cursor = _server_read_option(cursor, array_info[1]);
					printf("%s\n", array_info[1]);
					break;
				case 6:
					printf("DESTINO \n");
					cursor = _server_read_option(cursor, array_info[0]);
					printf("%s\n", array_info[0]);	
					break;
				case 2:
					printf("INTERFAZ \n");
					cursor = _server_read_option(cursor, array_info[2]);
					printf("%s\n", array_info[2]);	
					break;
				case 3:
					printf("METODO \n");
					cursor = _server_read_option(cursor, array_info[3]);
					printf("%s\n", array_info[3]);	
					break;
				case 8:
					printf("FIRMA \n");
					arguments_amount = _server_read_signature(cursor);
					printf("arg_amount:%d\n", arguments_amount);
					break;
			}
			i++;
		} else {
			(*cursor)++;
		}
	}

	return arguments_amount;
}

char** _server_read_option(char** cursor, char input_string[]){
	uint32_t option_len;

	(*cursor) += sizeof(uint32_t); //cursor apunta a long
	option_len = *((uint32_t*) (*cursor));
	printf("len: %u\n", option_len);

	(*cursor) += sizeof(uint32_t); //cursor apunta array
	for (int i = 0; i < option_len+1; ++i){
		printf("char: %c\n", **cursor);
		input_string[i]=(**cursor);
		(*cursor)++;
	}

	return cursor;
}

int _server_show(uint32_t header_info[], char array_info[][BUFF_SIZE], 
		char body_info[][BUFF_SIZE], int n_arguments){
	printf("* Id: 0x%08x\n", header_info[1]);
	printf("* Destino: %s\n", array_info[0]);
	printf("* Path: %s\n", array_info[1]);
	printf("* Interfaz: %s\n", array_info[2]);
	printf("* Método: %s\n", array_info[3]);
	if(n_arguments > 0){
		printf("* Parámetros:\n");
		for (int i = 0; i < n_arguments; ++i){
			printf("    * %s\n", body_info[i]);
		}
	}
	printf("\n");

	return 0;
}

int _server_read_signature(char** cursor){
	int n_arg;

	(*cursor) += sizeof(uint32_t);
	n_arg = **cursor;
	return n_arg;
}
					

int _server_response(server_t *self){
	char* response = "OK\n";

	socket_send(&(self->server_socket), response, strlen(response));

	return 0;
}

void server_destroy(server_t *self){
	socket_shutdown(&(self->server_socket), 2);
	socket_destroy(&(self->acep_socket));
	socket_destroy(&(self->server_socket));
}
