#ifndef TP1_SOCKET_H
#define TP1_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct socket_t{
	int socket_file_descriptor;
}socket_t;

int socket_create(socket_t *self);


/* Asigna una direccion al socket y lo marca como socket pasivo
(lado del servidor)
*/

int socket_bind_and_listen(socket_t *self, const char* service);

/*Espera a que se conecte un cliente y crea un nuevo fd, server_socket
que representa a la nueva conexión. 
/(lado del servidor)
*/

int socket_accept(socket_t *self, socket_t* server_socket);

/* Se conecta a un servidor dada una dirección y un servicio o puerto
(lado del cliente)
0 si ok -1 si hay error
*/
int socket_connect(socket_t *self, const char* host_name, const char* service);

/* Intenta enviar @param length bytes a traves del socket.
Devuelve la cantidad de bytes enviados o -1 si hay error.
*/
int socket_send(socket_t *self, const char* buffer, size_t length);

/*Intenta recibir @param length bytes a traves del socket.
Devuelve la cantidad de bytes recibidos o -1 si hay error.
*/

int socket_receive(socket_t *self, char* buffer, size_t length);


int socket_shutdown(socket_t *self, int channel);

int socket_destroy(socket_t *self);

#endif
