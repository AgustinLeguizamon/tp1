#ifndef TP1_CLIENT_H
#define TP1_CLIENT_H

#include "fileReader.h"
#include "socket.h"

typedef struct client_t{
	socket_t client_socket;
	file_reader_t file_reader;
}client_t;

/*LLama a los constructores de fileReader y socket e intenta
conectar el socket*/
int client_create(client_t *self, int argc, char const *argv[]);

/* Itera el llamado alectura del input en lineas, 
la aplicacion del protocolo dbus, envia el mensaje y
recibe una respuesta
*/
int client_run(client_t *self);


/*Muestra la respuesta del servidor con el numero de
@param id
*/
int _client_show(char* response, int id);

/*shutdown del canal de escritura y lectura y 
cierre del socket
*/
int client_destroy(client_t *self);

#endif
