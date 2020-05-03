#ifndef TP1_SERVER_H
#define TP1_SERVER_H

#include "socket.h"

#define BUFF_SIZE 130

typedef struct server_t{
	socket_t acep_socket;
	socket_t server_socket;
}server_t;


void server_create(server_t *self, char const* argv);

void server_run(server_t *self);

int _server_read_body(char* buffer, 
	char body_info[][BUFF_SIZE], int n_arguments);

char* _server_read_one_argument(char* cursor, char body_argument[]);

int _server_read_header(char* buffer, 
	uint32_t header_info[], char array_info[][BUFF_SIZE]);

int _server_read_header_signature(char** cursor, 
	uint32_t header_info[]);

int _server_read_header_array(char** cursor, 
	char array_info[][BUFF_SIZE], int body_len);

char** _server_read_option(char** cursor, char input_string[]);

int _server_read_signature(char** cursor);

int _server_show(uint32_t header_info[], 
	char array_info[][BUFF_SIZE], char body_info[][BUFF_SIZE], int n_arguments);

int _server_response(server_t *self);

void server_destroy(server_t *self);

#endif
