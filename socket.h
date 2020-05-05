#ifndef TP1_SOCKET_H
#define TP1_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct socket_t{
	int socket_file_descriptor;
}socket_t;

/*Inicializa el sfd en 0
*/

int socket_create(socket_t *self);

/* Asigna una direccion al socket del servidor y 
lo pone a la espera de una conexion
*/

int socket_bind_and_listen(socket_t *self, const char* service);

/*Espera a que se conecte un cliente y crea un nuevo sfd
que es el server_socket levantando el canal de comunicacion 
*/

int socket_accept(socket_t *self, socket_t* server_socket);

/* Se conecta a un servidor dada una dirección y un servicio
*/
int socket_connect(socket_t *self, const char* host_name, const char* service);

/* Envia cantidad length de bytes a traves del socket.
Devuelve la cantidad de bytes que se enviaron, 0 si el socket se cerro
o -1 si hubo un error.
*/
int socket_send(socket_t *self, const char* buffer, size_t length);

/* Recibe cantidad length de bytes a traves del socket.
Devuelve la cantidad de bytes que se recibieron, 0 si el socket se cerro
o -1 si hubo un error.*/

int socket_receive(socket_t *self, char* buffer, size_t length);

/*Cierra el canal de comunicacion
*/

int socket_shutdown(socket_t *self, int channel);

/*Cierra el socket
*/

int socket_destroy(socket_t *self);

#endif
