#ifndef TP1_CLIENT_H
#define TP1_CLIENT_H

#include "fileReader.h"
#include "socket.h"

typedef struct client_t{
	socket_t client_socket;
	file_reader_t file_reader;
}client_t;

void client_create(client_t *self, int argc, char const *argv[]);

int _client_connect_to_server(client_t *self, char* host_name, char* service);

void client_run(client_t *self);

int _client_show(char* response, int id);

void client_destroy(client_t *self);

#endif
