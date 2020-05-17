#ifndef TP1_SERVER_H
#define TP1_SERVER_H

#include "common_socket.h"

#define BUFF_SIZE 130

typedef struct server_t{
	socket_t acep_socket;
	socket_t server_socket;
}server_t;


/*Inicializa el socket 
*/
int server_create(server_t *self, char const* argv);

/*Itera el llamado a la recepcion de mensajes hasta que
se cierre el canal de comunicacion.Hace 3 recepciones 
(uno para la firma del header, el header y el cuerpo) y un envio
*/

int server_run(server_t *self);


/*Shutdown del canal de comunicacion y cierra los sockets acep y server.
*/

int server_destroy(server_t *self);

#endif
