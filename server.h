#ifndef TP1_SERVER_H
#define TP1_SERVER_H

#include "socket.h"

#define BUFF_SIZE 130

typedef struct server_t{
	socket_t acep_socket;
	socket_t server_socket;
}server_t;


/*Inicializa el socket 
*/
int server_create(server_t *self, char const* argv);

int server_run(server_t *self);

int _server_read_body(char* buffer, 
	char body_info[][BUFF_SIZE], int n_arguments);

char* _server_read_one_argument(char* cursor, char body_argument[]);

/*
*/


int _server_read_header(char* buffer, 
	uint32_t header_info[], char array_info[][BUFF_SIZE]);

/*Lee los primeros 16 bytes del header, guardando los 3 Uint32
en un array.
*/
int _server_read_header_signature(char** cursor, 
	uint32_t header_info[]);

/* Recorre todo el array del header con un cursor deteniendose 
en cada tipo de parametro y guardando su string en una matriz
*/

int _server_read_header_array(char** cursor, 
	char array_info[][BUFF_SIZE], int body_len);

/*Lee el string de uno del parametro al que este apuntando el
cursor del header y lo guarda en input_string*/

char** _server_read_option(char** cursor, char input_string[]);

/*Lee el uint32 del parametro firma 
correspondiente a la cantidad de argumentos del cuerpo */

int _server_read_signature(char** cursor);


/*Imprime por pantalla los datos que recivio por el socket
*/
int _server_show(uint32_t header_info[], 
	char array_info[][BUFF_SIZE], char body_info[][BUFF_SIZE], int n_arguments);

/*Envia el mensaje "OK\n* al client*/
int _server_send_response(server_t *self);

/*Shutdown del canal de comunicacion y cierra los sockets acep y server.
*/

int server_destroy(server_t *self);

#endif
